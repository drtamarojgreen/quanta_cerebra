#include "ui/terminal_renderer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <io.h>
#else
    #include <sys/ioctl.h>
    #include <termios.h>
    #include <unistd.h>
    #include <poll.h>
#endif

namespace cerebra {

TerminalSize terminal_size() {
    TerminalSize s;
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(h, &info)) {
        s.cols = info.srWindow.Right - info.srWindow.Left + 1;
        s.rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
#else
    winsize ws{};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        s.cols = ws.ws_col;
        s.rows = ws.ws_row;
    } else if (const char* env = std::getenv("COLUMNS")) {
        s.cols = std::atoi(env);
    }
#endif
    if (s.cols < 40) s.cols = 40;
    if (s.rows < 12) s.rows = 12;
    return s;
}

bool stdout_is_tty() {
#if defined(_WIN32)
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}

bool stdin_is_tty() {
#if defined(_WIN32)
    return _isatty(_fileno(stdin)) != 0;
#else
    return isatty(STDIN_FILENO) != 0;
#endif
}

bool color_supported() {
    if (!stdout_is_tty()) return false;
    const char* nc = std::getenv("NO_COLOR");
    if (nc && nc[0] != '\0') return false;
    const char* term = std::getenv("TERM");
    if (!term) return false;
    if (std::strcmp(term, "dumb") == 0) return false;
    return true;
}

std::string ansi(const std::string& sgr) {
    if (!color_supported()) return {};
    return "\x1b[" + sgr + "m";
}
std::string ansi_reset() { return color_supported() ? "\x1b[0m" : "; }
std::string ansi_clear_screen() { return "\x1b[2J\x1b[H"; }
std::string ansi_move(int row, int col) {
    return "\x1b[" + std::to_string(row) + ";" + std::to_string(col) + "H";
}
std::string ansi_hide_cursor() { return "\x1b[?25l"; }
std::string ansi_show_cursor() { return "\x1b[?25h"; }

char grayscale_block(double intensity) {
    if (intensity < 0) intensity = 0;
    if (intensity > 1) intensity = 1;
    static const char* gradient = " .:-=+*#%@";
    int idx = static_cast<int>(intensity * 9.999);
    if (idx < 0) idx = 0;
    if (idx > 9) idx = 9;
    return gradient[idx];
}

const char* utf8_intensity_block(double intensity) {
    if (intensity < 0) intensity = 0;
    if (intensity > 1) intensity = 1;
    static const char* blocks[] = {" ", "░", "▒", "▓", "█"};
    int idx = static_cast<int>(intensity * 4.999);
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return blocks[idx];
}

#if defined(_WIN32)

struct SavedState {
    DWORD in_mode = 0;
    DWORD out_mode = 0;
    UINT in_cp = 0;
    UINT out_cp = 0;
};

RawTerminal::RawTerminal() { saved_state_ = new SavedState{}; }
RawTerminal::~RawTerminal() { disable(); delete static_cast<SavedState*>(saved_state_); }

bool RawTerminal::enable() {
    if (active_) return true;
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    auto* st = static_cast<SavedState*>(saved_state_);
    if (!GetConsoleMode(hin, &st->in_mode)) return false;
    if (!GetConsoleMode(hout, &st->out_mode)) return false;
    st->in_cp = GetConsoleCP();
    st->out_cp = GetConsoleOutputCP();
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    DWORD nin = st->in_mode;
    nin &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    nin |= ENABLE_VIRTUAL_TERMINAL_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hin, nin);

    DWORD nout = st->out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hout, nout);

    active_ = true;
    return true;
}

void RawTerminal::disable() {
    if (!active_) return;
    enable_mouse(false);
    auto* st = static_cast<SavedState*>(saved_state_);
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleMode(hin, st->in_mode);
    SetConsoleMode(hout, st->out_mode);
    SetConsoleCP(st->in_cp);
    SetConsoleOutputCP(st->out_cp);
    active_ = false;
}

void RawTerminal::enable_mouse(bool on) {
    if (on == mouse_on_) return;
    if (on) std::cout << "\x1b[?1000h\x1b[?1006h" << std::flush;
    else    std::cout << "\x1b[?1006l\x1b[?1000l" << std::flush;
    mouse_on_ = on;
}

bool RawTerminal::poll(InputEvent& ev, int timeout_ms) {
    HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD rv = WaitForSingleObject(hin, static_cast<DWORD>(timeout_ms));
    if (rv != WAIT_OBJECT_0) return false;
    INPUT_RECORD rec;
    DWORD n = 0;
    if (!ReadConsoleInputA(hin, &rec, 1, &n) || n == 0) return false;
    if (rec.EventType == KEY_EVENT && rec.Event.KeyEvent.bKeyDown) {
        char c = rec.Event.KeyEvent.uChar.AsciiChar;
        WORD vk = rec.Event.KeyEvent.wVirtualKeyCode;
        if (vk == VK_UP)    { ev.key = Key::Up;    return true; }
        if (vk == VK_DOWN)  { ev.key = Key::Down;  return true; }
        if (vk == VK_LEFT)  { ev.key = Key::Left;  return true; }
        if (vk == VK_RIGHT) { ev.key = Key::Right; return true; }
        if (c == 27)        { ev.key = Key::Escape; return true; }
        if (c == 13 || c == 10) { ev.key = Key::Enter; return true; }
        if (c == 8 || c == 127) { ev.key = Key::Backspace; return true; }
        if (c == ' ')           { ev.key = Key::Space; ev.ch = ' '; return true; }
        if (c == '\t')          { ev.key = Key::Tab; return true; }
        if (c != 0)             { ev.key = Key::Char; ev.ch = c; return true; }
    } else if (rec.EventType == MOUSE_EVENT) {
        ev.key = Key::Mouse;
        ev.mouse_x = rec.Event.MouseEvent.dwMousePosition.X + 1;
        ev.mouse_y = rec.Event.MouseEvent.dwMousePosition.Y + 1;
        ev.mouse_button = (rec.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? 1 : 0;
        return true;
    }
    return false;
}

#else // POSIX

struct SavedState {
    termios tio{};
    bool valid = false;
};

RawTerminal::RawTerminal() { saved_state_ = new SavedState{}; }
RawTerminal::~RawTerminal() { disable(); delete static_cast<SavedState*>(saved_state_); }

bool RawTerminal::enable() {
    if (active_) return true;
    if (!isatty(STDIN_FILENO)) return false;
    auto* st = static_cast<SavedState*>(saved_state_);
    if (tcgetattr(STDIN_FILENO, &st->tio) != 0) return false;
    st->valid = true;
    termios raw = st->tio;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) return false;
    active_ = true;
    return true;
}

void RawTerminal::disable() {
    if (!active_) return;
    enable_mouse(false);
    auto* st = static_cast<SavedState*>(saved_state_);
    if (st->valid) tcsetattr(STDIN_FILENO, TCSANOW, &st->tio);
    active_ = false;
}

void RawTerminal::enable_mouse(bool on) {
    if (on == mouse_on_) return;
    if (on) std::cout << "\x1b[?1000h\x1b[?1006h" << std::flush;
    else    std::cout << "\x1b[?1006l\x1b[?1000l" << std::flush;
    mouse_on_ = on;
}

bool RawTerminal::poll(InputEvent& ev, int timeout_ms) {
    pollfd pfd{STDIN_FILENO, POLLIN, 0};
    int rv = ::poll(&pfd, 1, timeout_ms);
    if (rv <= 0) return false;
    char c;
    ssize_t n = ::read(STDIN_FILENO, &c, 1);
    if (n <= 0) return false;
    if (c == 27) {
        // possible escape sequence
        pollfd pf2{STDIN_FILENO, POLLIN, 0};
        if (::poll(&pf2, 1, 10) <= 0) {
            ev.key = Key::Escape;
            return true;
        }
        char seq[64];
        ssize_t sn = ::read(STDIN_FILENO, seq, sizeof(seq));
        if (sn <= 0) { ev.key = Key::Escape; return true; }
        if (seq[0] == '[') {
            if (sn >= 2 && seq[1] == 'A') { ev.key = Key::Up; return true; }
            if (sn >= 2 && seq[1] == 'B') { ev.key = Key::Down; return true; }
            if (sn >= 2 && seq[1] == 'C') { ev.key = Key::Right; return true; }
            if (sn >= 2 && seq[1] == 'D') { ev.key = Key::Left; return true; }
            if (sn >= 2 && seq[1] == '<') {
                // SGR mouse: ESC [ < b ; x ; y M/m
                std::string body(seq + 2, seq + sn);
                int b = 0, x = 0, y = 0;
                char term = 0;
                int parsed = std::sscanf(body.c_str(), "%d;%d;%d%c", &b, &x, &y, &term);
                if (parsed == 4) {
                    ev.key = Key::Mouse;
                    ev.mouse_button = (term == 'M') ? (b + 1) : 0;
                    ev.mouse_x = x;
                    ev.mouse_y = y;
                    return true;
                }
            }
        }
        ev.key = Key::Escape;
        return true;
    }
    if (c == '\n' || c == '\r') { ev.key = Key::Enter; return true; }
    if (c == 127 || c == 8)     { ev.key = Key::Backspace; return true; }
    if (c == '\t')              { ev.key = Key::Tab; return true; }
    if (c == ' ')               { ev.key = Key::Space; ev.ch = ' '; return true; }
    ev.key = Key::Char;
    ev.ch = c;
    return true;
}

#endif

}
