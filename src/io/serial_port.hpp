#ifndef BRAIN_MODELER_SERIAL_PORT_HPP
#define BRAIN_MODELER_SERIAL_PORT_HPP

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "core/sample.hpp"

namespace cerebra {

struct SerialConfig {
  std::string device;        // e.g. "/dev/ttyUSB0" or "COM3"
  int baud_rate = 115200;
  int read_timeout_ms = 50;  // per-read deadline
};

// Abstract byte-stream serial port. Concrete implementations use the native OS
// API: termios on POSIX, the Win32 COM API on Windows. Tests can substitute a
// MemorySerialPort that is fed bytes programmatically.
class SerialPort {
public:
  virtual ~SerialPort() = default;

  virtual bool open(const SerialConfig& config) = 0;
  virtual void close() = 0;
  virtual bool is_open() const = 0;

  // Read up to max_bytes; returns the bytes actually read (may be empty on
  // timeout). Throws std::runtime_error on a hard I/O error.
  virtual std::vector<std::uint8_t> read(std::size_t max_bytes) = 0;
  virtual std::size_t write(const std::uint8_t* data, std::size_t len) = 0;
  std::size_t write_str(const std::string& s) {
    return write(reinterpret_cast<const std::uint8_t*>(s.data()), s.size());
  }

  // Factory for the platform-native implementation.
  static std::unique_ptr<SerialPort> create_native();
};

// In-memory port for tests and offline simulation of an IoT device.
class MemorySerialPort : public SerialPort {
public:
  bool open(const SerialConfig&) override { open_ = true; return true; }
  void close() override { open_ = false; }
  bool is_open() const override { return open_; }
  std::vector<std::uint8_t> read(std::size_t max_bytes) override;
  std::size_t write(const std::uint8_t* data, std::size_t len) override;

  // Test helpers: queue bytes the application will "receive".
  void feed(const std::string& text);
  void feed(const std::vector<std::uint8_t>& bytes);
  // Inspect what the application "sent" (cumulative).
  const std::string& sent() const { return sent_; }
  // Drain (return and clear) what the application has sent since the last call;
  // used by the device side of a simulated link.
  std::string take_sent();

private:
  bool open_ = false;
  std::deque<std::uint8_t> rx_;
  std::string sent_;
};

// Decodes a line-delimited stream of JSON activity frames coming over a serial
// link from an experimental device. Each line is one frame in the project's
// format: {"brain_activity":[{"region":..,"intensity":..},..],"timestamp_ms":N}
//
// Also supports a tiny request protocol: request_state("focused") writes
// "STATE focused\n" to the device, which firmware may use to switch presets.
class SerialActivityStream {
public:
  explicit SerialActivityStream(std::unique_ptr<SerialPort> port);

  bool open(const SerialConfig& config) { return port_->open(config); }
  void close() { port_->close(); }
  bool is_open() const { return port_->is_open(); }
  SerialPort& port() { return *port_; }

  // Pull whatever bytes are waiting and return any complete frames decoded.
  // Lines that fail to parse are skipped (and counted in parse_errors()).
  std::vector<BrainActivitySample> poll();

  // Parse a single text line into a sample (exposed for testing).
  static std::optional<BrainActivitySample> parse_line(const std::string& line);

  // Send a newline-terminated JSON payload to the device.
  void send_json(const JsonValue& payload);
  // Convenience: ask the device to switch to a brain-state preset by sending
  // {"command":"set_state","state":"<key>"}.
  void request_state(const std::string& state_key);

  std::size_t frames_decoded() const { return frames_decoded_; }
  std::size_t parse_errors() const { return parse_errors_; }

private:
  std::unique_ptr<SerialPort> port_;
  std::string buffer_;
  std::size_t frames_decoded_ = 0;
  std::size_t parse_errors_ = 0;
};

}  // namespace cerebra

#endif  // BRAIN_MODELER_SERIAL_PORT_HPP
