#include "include/idt.h"
#include "include/io.h"

extern "C" struct idt_entry_struct idt[256];
struct idt_entry_struct idt;

// Функции из boot.asm
extern "C" void load_idt();
extern "C" void keyboard_handler_asm();
extern "C" void mouse_handler_asm(); // Добавь эту метку в boot.asm!
extern "C" void irq_stub();

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_lo = (base & 0xFFFF);
    idt[num].base_hi = (base >> 16) & 0xFFFF;
    idt[num].sel     = sel;
    idt[num].always0 = 0;
    idt[num].flags   = flags;
}

void pic_remap() {
    // Инициализация
    outb(0x20, 0x11); outb(0xA0, 0x11);
    // Смещение векторов (32-47)
    outb(0x21, 0x20); outb(0xA1, 0x28);
    // Каскадирование
    outb(0x21, 0x04); outb(0xA1, 0x02);
    // Режим 8086
    outb(0x21, 0x01); outb(0xA1, 0x01);
    
    // МАСКИРОВАНИЕ: Разрешаем клавиатуру (бит 1) и Slave PIC (бит 2) на Master
    // На Slave разрешаем мышь (бит 4, так как IRQ12 - это 4-й пин Slave)
    outb(0x21, 0xF9); // 11111001 (разрешены IRQ1 и IRQ2)
    outb(0xA1, 0xEF); // 11101111 (разрешен IRQ12)
}

extern "C" void init_idt() {
    // Заполняем всё заглушками
    for(int i = 0; i < 256; i++) {
        idt_set_gate(i, (uint32_t)irq_stub, 0x08, 0x8E);
    }

    pic_remap();

    // Клавиатура (IRQ1 -> Вектор 0x21)
    idt_set_gate(0x21, (uint32_t)keyboard_handler_asm, 0x08, 0x8E);

    // Мышь (IRQ12 -> Вектор 0x2C)
    // 0x28 (база Slave) + 4 (IRQ12) = 0x2C
    idt_set_gate(0x2C, (uint32_t)mouse_handler_asm, 0x08, 0x8E);

    load_idt();
    __asm__ __volatile__ ("sti");
}
