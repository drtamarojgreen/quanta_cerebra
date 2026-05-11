#pragma once

#include <chrono>
#include <cstddef>
#include <string>

namespace cerebra {

// Cross-platform serial port using native OS APIs:
//   * Linux: POSIX termios on a /dev/tty* file descriptor
//   * Windows: CreateFile + SetCommState / SetCommTimeouts
class SerialPort {
public:
    SerialPort();
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    SerialPort(SerialPort&& other) noexcept;
    SerialPort& operator=(SerialPort&& other) noexcept;

    // device example: "/dev/ttyUSB0" on Linux or "COM3" / "\\\\.\\COM3" on Windows.
    bool open(const std::string& device, int baud = 115200);
    void close();
    bool is_open() const;

    // Returns bytes written (or -1 on error). Best-effort, may write less than asked.
    long write(const void* data, std::size_t n);

    // Reads up to n bytes, blocking up to timeout. Returns bytes read, 0 on timeout, -1 on error.
    long read(void* buf, std::size_t n, std::chrono::milliseconds timeout);

    // Read a single newline-terminated line (newline stripped). Returns false on timeout/error.
    bool read_line(std::string& out, std::chrono::milliseconds timeout);

    const std::string& device() const { return device_; }
    const std::string& last_error() const { return last_error_; }

private:
    void* native_handle_ = nullptr; // platform-specific handle storage
    bool open_ = false;
    std::string device_;
    std::string last_error_;
    std::string read_buf_;
};

}
