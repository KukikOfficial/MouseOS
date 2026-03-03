#ifndef IO_H
#define IO_H

#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

// Функция мягкой перезагрузки через контроллер 8042
static inline void sys_reboot() {
    uint8_t temp;
    // Ожидаем, пока входной буфер контроллера освободится
    do {
        temp = inb(0x64);
    } while (temp & 0x02);
    // Команда 0xFE — Pulse Reset
    outb(0x64, 0xFE);
}

#endif
