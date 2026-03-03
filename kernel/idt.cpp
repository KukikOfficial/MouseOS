#include "include/idt.h"
#include "include/io.h"

extern "C" struct idt_entry_struct idt[256]; // Важно: массив!

extern "C" void load_idt();
extern "C" void keyboard_handler_asm();

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void pic_remap() {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x21, 0x04); outb(0xA1, 0x02);
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    // Разрешаем только клавиатуру
    outb(0x21, 0xFD); 
    outb(0xA1, 0xFF);
}

extern "C" void init_idt() {
    // Чистим таблицу (ставим пустые заглушки не будем, чтобы не усложнять)
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Устанавливаем клавиатуру на вектор 0x21
    // Теперь 0x08 точно указывает на наш gdt_code
    idt_set_gate(0x21, (uint32_t)keyboard_handler_asm, 0x08, 0x8E);

    pic_remap();
    load_idt();

    // Пробуем включить!
    __asm__ __volatile__ ("sti");
}
