#include "include/idt.h"
#include "include/io.h"

// Объявляем массив IDT. 
// extern "C" нужен, чтобы ассемблер видел это имя без искажений.
extern "C" struct idt_entry_struct idt[256];
struct idt_entry_struct idt[256]; // Само определение массива

// Внешние функции из boot.asm
extern "C" void load_idt();
extern "C" void keyboard_handler_asm();
extern "C" void mouse_handler_asm();
extern "C" void irq_stub();

// Установка ворот в таблице
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

// Перепрошивка PIC (как мы делали раньше)
void pic_remap() {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    // Разрешаем: IRQ1 (клавиатура), IRQ2 (каскад), IRQ12 (мышь)
    outb(0x21, 0xF9); // 11111001
    outb(0xA1, 0xEF); // 11101111
}

extern "C" void init_idt() {
    // Заполняем всё заглушками irq_stub
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)irq_stub, 0x08, 0x8E);
    }

    pic_remap();

    // Ставим обработчики
    idt_set_gate(0x21, (uint32_t)keyboard_handler_asm, 0x08, 0x8E);
    idt_set_gate(0x2C, (uint32_t)mouse_handler_asm, 0x08, 0x8E);

    load_idt();
    __asm__ __volatile__ ("sti");
}
