#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Оборачиваем для совместимости имен C и C++
extern "C" {
    void init_mouse_driver();
    void mouse_handler_main();
    void update_mouse_cursor();
}

#endif
