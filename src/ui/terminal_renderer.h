#pragma once

#include "ui/visual_themes.h"

#include <cstddef>
#include <optional>
#include <string>

namespace cerebra {

struct TerminalSize {
    int cols = 80;
    int rows = 24;
};

TerminalSize terminal_size();

bool stdout_is_tty();
bool stdin_is_tty();
bool color_supported();

enum class Key {
    None,
    Char,
    Up, Down, Left, Right,
    Enter,
    Escape,
    Backspace,
    Space,
    Tab,
    Mouse,
};

struct InputEvent {
    Key key = Key::None;
    char ch = 0;
    int mouse_button = 0; // 0 = release
    int mouse_x = 0;
    int mouse_y = 0;
};

// RAII raw-mode terminal for interactive input.
class RawTerminal {
public:
    RawTerminal();
    ~RawTerminal();
    RawTerminal(const RawTerminal&) = delete;
    RawTerminal& operator=(const RawTerminal&) = delete;

    bool enable();
    void disable();
    bool enabled() const { return active_; }

    void enable_mouse(bool on);

    // Reads an input event with timeout. Returns false on timeout.
    bool poll(InputEvent& ev, int timeout_ms);

private:
    bool active_ = false;
    bool mouse_on_ = false;
    void* saved_state_ = nullptr;
};

std::string ansi(const std::string& sgr);
std::string ansi_reset();
std::string ansi_clear_screen();
std::string ansi_move(int row, int col);
std::string ansi_hide_cursor();
std::string ansi_show_cursor();

// Map [0,1] intensity to a grayscale block character.
char grayscale_block(double intensity);

// Map [0,1] intensity to an extended UTF-8 block string (heavier with more density).
const char* utf8_intensity_block(double intensity);

}
