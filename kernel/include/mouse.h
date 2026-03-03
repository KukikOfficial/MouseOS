#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Инициализация PS/2 контроллера для мыши
void init_mouse_driver();

// Функция отрисовки курсора (вызывается в цикле или прерывании)
void update_mouse_cursor();

// Основной обработчик прерывания (для boot.asm)
extern "C" void mouse_handler_main();

#endif
