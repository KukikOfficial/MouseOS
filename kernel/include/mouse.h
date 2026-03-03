#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

extern "C" {
    void init_mouse_driver();
    void update_mouse_cursor();
    void mouse_handler_main();
}

#endif
