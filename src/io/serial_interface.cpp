#include "io/serial_interface.h"

#include <cstdint>
#include <cstring>
#include <utility>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <errno.h>
    #include <fcntl.h>
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#endif

namespace cerebra {

namespace {

#if defined(_WIN32)
    constexpr std::uintptr_t kInvalid = reinterpret_cast<std::uintptr_t>(INVALID_HANDLE_VALUE);
    HANDLE as_handle(void* p) { return reinterpret_cast<HANDLE>(p); }
    void* from_handle(HANDLE h) { return reinterpret_cast<void*>(h); }
#else
    constexpr std::intptr_t kInvalid = -1;
    int as_fd(void* p) { return static_cast<int>(reinterpret_cast<std::intptr_t>(p)); }
    void* from_fd(int fd) { return reinterpret_cast<void*>(static_cast<std::intptr_t>(fd)); }
#endif

}

SerialPort::SerialPort() {
#if defined(_WIN32)
    native_handle_ = from_handle(INVALID_HANDLE_VALUE);
#else
    native_handle_ = from_fd(-1);
#endif
}

SerialPort::~SerialPort() { close(); }

SerialPort::SerialPort(SerialPort&& other) noexcept
    : native_handle_(other.native_handle_),
      open_(other.open_),
      device_(std::move(other.device_)),
      last_error_(std::move(other.last_error_)),
      read_buf_(std::move(other.read_buf_)) {
#if defined(_WIN32)
    other.native_handle_ = from_handle(INVALID_HANDLE_VALUE);
#else
    other.native_handle_ = from_fd(-1);
#endif
    other.open_ = false;
}

SerialPort& SerialPort::operator=(SerialPort&& other) noexcept {
    if (this != &other) {
        close();
        native_handle_ = other.native_handle_;
        open_ = other.open_;
        device_ = std::move(other.device_);
        last_error_ = std::move(other.last_error_);
        read_buf_ = std::move(other.read_buf_);
#if defined(_WIN32)
        other.native_handle_ = from_handle(INVALID_HANDLE_VALUE);
#else
        other.native_handle_ = from_fd(-1);
#endif
        other.open_ = false;
    }
    return *this;
}

bool SerialPort::is_open() const { return open_; }

#if defined(_WIN32)

bool SerialPort::open(const std::string& device, int baud) {
    close();
    device_ = device;

    std::string name = device;
    if (name.rfind("\\\\.\\", 0) != 0 && name.size() > 0 && name[0] != '\\') {
        name = "\\\\.\\" + name;
    }

    HANDLE h = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        last_error_ = "CreateFile failed";
        return false;
    }

    DCB dcb{};
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(h, &dcb)) {
        last_error_ = "GetCommState failed";
        CloseHandle(h);
        return false;
    }
    dcb.BaudRate = static_cast<DWORD>(baud);
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fParity = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    if (!SetCommState(h, &dcb)) {
        last_error_ = "SetCommState failed";
        CloseHandle(h);
        return false;
    }

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = MAXDWORD;
    to.ReadTotalTimeoutConstant = 0;
    to.ReadTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant = 100;
    to.WriteTotalTimeoutMultiplier = 1;
    SetCommTimeouts(h, &to);

    native_handle_ = from_handle(h);
    open_ = true;
    return true;
}

void SerialPort::close() {
    HANDLE h = as_handle(native_handle_);
    if (h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
        native_handle_ = from_handle(INVALID_HANDLE_VALUE);
    }
    open_ = false;
}

long SerialPort::write(const void* data, std::size_t n) {
    if (!open_) return -1;
    DWORD written = 0;
    if (!WriteFile(as_handle(native_handle_), data, static_cast<DWORD>(n), &written, nullptr)) {
        last_error_ = "WriteFile failed";
        return -1;
    }
    return static_cast<long>(written);
}

long SerialPort::read(void* buf, std::size_t n, std::chrono::milliseconds timeout) {
    if (!open_) return -1;
    HANDLE h = as_handle(native_handle_);

    COMMTIMEOUTS to{};
    to.ReadIntervalTimeout = MAXDWORD;
    to.ReadTotalTimeoutConstant = static_cast<DWORD>(timeout.count());
    to.ReadTotalTimeoutMultiplier = 0;
    SetCommTimeouts(h, &to);

    DWORD got = 0;
    if (!ReadFile(h, buf, static_cast<DWORD>(n), &got, nullptr)) {
        last_error_ = "ReadFile failed";
        return -1;
    }
    return static_cast<long>(got);
}

#else // POSIX

bool SerialPort::open(const std::string& device, int baud) {
    close();
    device_ = device;
    int fd = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        last_error_ = std::string("open failed: ") + std::strerror(errno);
        return false;
    }
    termios tio{};
    if (tcgetattr(fd, &tio) != 0) {
        last_error_ = std::string("tcgetattr failed: ") + std::strerror(errno);
        ::close(fd);
        return false;
    }
    cfmakeraw(&tio);
    tio.c_cflag |= (CLOCAL | CREAD);
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 0;

    speed_t s = B115200;
    switch (baud) {
        case 9600:   s = B9600;   break;
        case 19200:  s = B19200;  break;
        case 38400:  s = B38400;  break;
        case 57600:  s = B57600;  break;
        case 115200: s = B115200; break;
        case 230400: s = B230400; break;
        default: break;
    }
    cfsetispeed(&tio, s);
    cfsetospeed(&tio, s);

    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        last_error_ = std::string("tcsetattr failed: ") + std::strerror(errno);
        ::close(fd);
        return false;
    }
    native_handle_ = from_fd(fd);
    open_ = true;
    return true;
}

void SerialPort::close() {
    int fd = as_fd(native_handle_);
    if (fd >= 0) {
        ::close(fd);
        native_handle_ = from_fd(-1);
    }
    open_ = false;
}

long SerialPort::write(const void* data, std::size_t n) {
    if (!open_) return -1;
    ssize_t w = ::write(as_fd(native_handle_), data, n);
    if (w < 0) {
        last_error_ = std::string("write failed: ") + std::strerror(errno);
    }
    return static_cast<long>(w);
}

long SerialPort::read(void* buf, std::size_t n, std::chrono::milliseconds timeout) {
    if (!open_) return -1;
    int fd = as_fd(native_handle_);

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    timeval tv;
    tv.tv_sec  = static_cast<long>(timeout.count() / 1000);
    tv.tv_usec = static_cast<long>((timeout.count() % 1000) * 1000);
    int rv = select(fd + 1, &rfds, nullptr, nullptr, &tv);
    if (rv == 0) return 0;
    if (rv < 0) {
        last_error_ = std::string("select failed: ") + std::strerror(errno);
        return -1;
    }
    ssize_t r = ::read(fd, buf, n);
    if (r < 0) {
        last_error_ = std::string("read failed: ") + std::strerror(errno);
        return -1;
    }
    return static_cast<long>(r);
}

#endif // _WIN32

bool SerialPort::read_line(std::string& out, std::chrono::milliseconds timeout) {
    out.clear();
    auto deadline = std::chrono::steady_clock::now() + timeout;
    char buf[256];
    while (true) {
        auto nl = read_buf_.find('\n');
        if (nl != std::string::npos) {
            out.assign(read_buf_, 0, nl);
            if (!out.empty() && out.back() == '\r') out.pop_back();
            read_buf_.erase(0, nl + 1);
            return true;
        }
        auto now = std::chrono::steady_clock::now();
        if (now >= deadline) return false;
        auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now);
        long n = read(buf, sizeof(buf), remaining);
        if (n < 0) return false;
        if (n == 0) continue;
        read_buf_.append(buf, static_cast<std::size_t>(n));
    }
}

}
