#include "io/serial_port.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#include "core/region.hpp"

#if defined(BRAIN_MODELER_PLATFORM_WINDOWS)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <errno.h>
#  include <fcntl.h>
#  include <termios.h>
#  include <unistd.h>
#  include <sys/select.h>
#endif

namespace cerebra {

// ---------------------------------------------------------------------------
// MemorySerialPort
// ---------------------------------------------------------------------------

std::vector<std::uint8_t> MemorySerialPort::read(std::size_t max_bytes) {
  std::vector<std::uint8_t> out;
  while (!rx_.empty() && out.size() < max_bytes) {
    out.push_back(rx_.front());
    rx_.pop_front();
  }
  return out;
}

std::size_t MemorySerialPort::write(const std::uint8_t* data, std::size_t len) {
  sent_.append(reinterpret_cast<const char*>(data), len);
  return len;
}

void MemorySerialPort::feed(const std::string& text) {
  for (char c : text) rx_.push_back(static_cast<std::uint8_t>(c));
}

void MemorySerialPort::feed(const std::vector<std::uint8_t>& bytes) {
  for (auto b : bytes) rx_.push_back(b);
}

std::string MemorySerialPort::take_sent() {
  std::string out = std::move(sent_);
  sent_.clear();
  return out;
}

// ---------------------------------------------------------------------------
// Native serial port implementations
// ---------------------------------------------------------------------------

#if defined(BRAIN_MODELER_PLATFORM_WINDOWS)

class WindowsSerialPort : public SerialPort {
public:
  ~WindowsSerialPort() override { close(); }

  bool open(const SerialConfig& config) override {
    close();
    std::string name = config.device;
    // COM ports above 9 need the \\.\ prefix.
    if (name.rfind("\\\\.\\", 0) != 0) name = "\\\\.\\" + name;
    handle_ = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                          OPEN_EXISTING, 0, nullptr);
    if (handle_ == INVALID_HANDLE_VALUE) { handle_ = nullptr; return false; }

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(handle_, &dcb)) { close(); return false; }
    dcb.BaudRate = static_cast<DWORD>(config.baud_rate);
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    if (!SetCommState(handle_, &dcb)) { close(); return false; }

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = MAXDWORD;
    to.ReadTotalTimeoutConstant = static_cast<DWORD>(std::max(1, config.read_timeout_ms));
    to.ReadTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant = 100;
    to.WriteTotalTimeoutMultiplier = 0;
    SetCommTimeouts(handle_, &to);
    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return true;
  }

  void close() override {
    if (handle_) { CloseHandle(handle_); handle_ = nullptr; }
  }
  bool is_open() const override { return handle_ != nullptr; }

  std::vector<std::uint8_t> read(std::size_t max_bytes) override {
    if (!handle_) throw std::runtime_error("serial port is not open");
    std::vector<std::uint8_t> buf(max_bytes);
    DWORD got = 0;
    if (!ReadFile(handle_, buf.data(), static_cast<DWORD>(max_bytes), &got, nullptr)) {
      throw std::runtime_error("serial read failed");
    }
    buf.resize(got);
    return buf;
  }

  std::size_t write(const std::uint8_t* data, std::size_t len) override {
    if (!handle_) throw std::runtime_error("serial port is not open");
    DWORD wrote = 0;
    if (!WriteFile(handle_, data, static_cast<DWORD>(len), &wrote, nullptr)) {
      throw std::runtime_error("serial write failed");
    }
    return wrote;
  }

private:
  HANDLE handle_ = nullptr;
};

#else  // POSIX

class PosixSerialPort : public SerialPort {
public:
  ~PosixSerialPort() override { close(); }

  bool open(const SerialConfig& config) override {
    close();
    fd_ = ::open(config.device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) return false;

    termios tio{};
    if (tcgetattr(fd_, &tio) != 0) { close(); return false; }
    cfmakeraw(&tio);
    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~static_cast<tcflag_t>(CSIZE);
    tio.c_cflag |= CS8;
    tio.c_cflag &= ~static_cast<tcflag_t>(PARENB);
    tio.c_cflag &= ~static_cast<tcflag_t>(CSTOPB);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;
    speed_t speed = to_speed(config.baud_rate);
    cfsetispeed(&tio, speed);
    cfsetospeed(&tio, speed);
    if (tcsetattr(fd_, TCSANOW, &tio) != 0) { close(); return false; }
    tcflush(fd_, TCIOFLUSH);
    timeout_ms_ = std::max(0, config.read_timeout_ms);
    return true;
  }

  void close() override {
    if (fd_ >= 0) { ::close(fd_); fd_ = -1; }
  }
  bool is_open() const override { return fd_ >= 0; }

  std::vector<std::uint8_t> read(std::size_t max_bytes) override {
    if (fd_ < 0) throw std::runtime_error("serial port is not open");
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd_, &set);
    timeval tv{};
    tv.tv_sec = timeout_ms_ / 1000;
    tv.tv_usec = (timeout_ms_ % 1000) * 1000;
    int r = ::select(fd_ + 1, &set, nullptr, nullptr, &tv);
    if (r < 0) {
      if (errno == EINTR) return {};
      throw std::runtime_error(std::string("serial select failed: ") + std::strerror(errno));
    }
    if (r == 0) return {};
    std::vector<std::uint8_t> buf(max_bytes);
    ssize_t n = ::read(fd_, buf.data(), max_bytes);
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return {};
      throw std::runtime_error(std::string("serial read failed: ") + std::strerror(errno));
    }
    buf.resize(static_cast<std::size_t>(n));
    return buf;
  }

  std::size_t write(const std::uint8_t* data, std::size_t len) override {
    if (fd_ < 0) throw std::runtime_error("serial port is not open");
    ssize_t n = ::write(fd_, data, len);
    if (n < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
      throw std::runtime_error(std::string("serial write failed: ") + std::strerror(errno));
    }
    return static_cast<std::size_t>(n);
  }

private:
  static speed_t to_speed(int baud) {
    switch (baud) {
      case 9600: return B9600;
      case 19200: return B19200;
      case 38400: return B38400;
      case 57600: return B57600;
      case 115200: return B115200;
      case 230400: return B230400;
#ifdef B460800
      case 460800: return B460800;
#endif
#ifdef B921600
      case 921600: return B921600;
#endif
      default: return B115200;
    }
  }

  int fd_ = -1;
  int timeout_ms_ = 50;
};

#endif

std::unique_ptr<SerialPort> SerialPort::create_native() {
#if defined(BRAIN_MODELER_PLATFORM_WINDOWS)
  return std::make_unique<WindowsSerialPort>();
#else
  return std::make_unique<PosixSerialPort>();
#endif
}

// ---------------------------------------------------------------------------
// SerialActivityStream
// ---------------------------------------------------------------------------

SerialActivityStream::SerialActivityStream(std::unique_ptr<SerialPort> port)
    : port_(std::move(port)) {}

std::optional<BrainActivitySample> SerialActivityStream::parse_line(const std::string& line) {
  // Trim whitespace.
  std::size_t b = line.find_first_not_of(" \t\r\n");
  if (b == std::string::npos) return std::nullopt;
  std::size_t e = line.find_last_not_of(" \t\r\n");
  std::string trimmed = line.substr(b, e - b + 1);
  if (trimmed.empty() || trimmed[0] != '{') return std::nullopt;
  try {
    JsonValue v = JsonValue::parse(trimmed);
    if (!v.is_object()) return std::nullopt;
    BrainActivitySample sample;
    sample.timestamp_ms = v["timestamp_ms"].as_int(0);
    const JsonValue& act = v["brain_activity"];
    if (!act.is_array()) return std::nullopt;
    for (const auto& entry : act.as_array()) {
      const std::string& raw = entry["region"].as_string();
      if (raw.empty()) continue;
      double intensity = std::max(0.0, std::min(1.0, entry["intensity"].as_number(0.0)));
      sample.intensities[RegionCatalog::normalize_key(raw)] = intensity;
    }
    return sample;
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

std::vector<BrainActivitySample> SerialActivityStream::poll() {
  std::vector<BrainActivitySample> out;
  if (!port_->is_open()) return out;
  auto bytes = port_->read(4096);
  buffer_.append(reinterpret_cast<const char*>(bytes.data()), bytes.size());

  std::size_t pos;
  while ((pos = buffer_.find('\n')) != std::string::npos) {
    std::string line = buffer_.substr(0, pos);
    buffer_.erase(0, pos + 1);
    auto sample = parse_line(line);
    if (sample) {
      ++frames_decoded_;
      out.push_back(std::move(*sample));
    } else if (line.find_first_not_of(" \t\r\n") != std::string::npos &&
               line.find('{') != std::string::npos) {
      ++parse_errors_;
    }
  }
  // Guard against an unbounded buffer if the device never sends a newline.
  if (buffer_.size() > (1u << 20)) buffer_.clear();
  return out;
}

void SerialActivityStream::send_json(const JsonValue& payload) {
  if (!port_->is_open()) return;
  port_->write_str(payload.dump() + "\n");
}

void SerialActivityStream::request_state(const std::string& state_key) {
  JsonValue::Object cmd;
  cmd.emplace("command", JsonValue("set_state"));
  cmd.emplace("state", JsonValue(state_key));
  send_json(JsonValue(std::move(cmd)));
}

}  // namespace cerebra
