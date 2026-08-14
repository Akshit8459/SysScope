#include "sysscope/ui/terminal.hpp"
#include <iostream>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#endif

namespace sysscope::ui {

#if !defined(_WIN32) && !defined(_WIN64)
static struct termios orig_termios;
static bool raw_mode_enabled = false;
#endif

void Terminal::enable_raw_mode() {
#if !defined(_WIN32) && !defined(_WIN64)
    if (raw_mode_enabled) return;
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON));
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_enabled = true;
#endif
    // Enter alternate screen buffer & hide cursor for clean in-place TUI rendering
    std::cout << "\033[?1049h\033[?25l\033[H\033[2J" << std::flush;
}

void Terminal::disable_raw_mode() {
#if !defined(_WIN32) && !defined(_WIN64)
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_enabled = false;
    }
#endif
    // Show cursor & leave alternate screen buffer
    std::cout << "\033[?25h\033[?1049l" << std::flush;
}

void Terminal::clear_screen() {
    std::cout << "\033[H\033[J" << std::flush;
}

void Terminal::move_cursor(int row, int col) {
    std::cout << "\033[" << row << ";" << col << "H";
}

bool Terminal::has_key_pressed() {
#if defined(_WIN32) || defined(_WIN64)
    return _kbhit() != 0;
#else
    struct timeval tv{0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0;
#endif
}

char Terminal::read_key() {
#if defined(_WIN32) || defined(_WIN64)
    if (_kbhit()) return static_cast<char>(_getch());
    return '\0';
#else
    if (has_key_pressed()) {
        char ch = 0;
        if (read(STDIN_FILENO, &ch, 1) > 0) return ch;
    }
    return '\0';
#endif
}

} // namespace sysscope::ui
