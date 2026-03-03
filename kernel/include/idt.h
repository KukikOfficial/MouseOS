#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Структура записи IDT (8 байт)
struct idt_entry_struct {
    uint16_t base_lo;             // Младшие 16 бит адреса обработчика
    uint16_t sel;                 // Селектор сегмента кода (обычно 0x08)
    uint8_t  always0;             // Всегда 0
    uint8_t  flags;               // Флаги (тип шлюза, уровень доступа)
    uint16_t base_hi;             // Старшие 16 бит адреса
} __attribute__((packed));

// Структура, которую понимает процессор (IDTR)
struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern "C" {
    void init_idt();
}

#endif
