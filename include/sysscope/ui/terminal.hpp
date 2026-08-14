#pragma once

#include <string>

namespace sysscope::ui {

class Terminal {
public:
    static void enable_raw_mode();
    static void disable_raw_mode();
    static void clear_screen();
    static void move_cursor(int row, int col);
    static bool has_key_pressed();
    static char read_key();
};

} // namespace sysscope::ui
