#include "include/vesa.h"
#include "include/font.h"
#include "include/mouse.h"
#include "include/idt.h"
#include "include/fs.h"
#include "include/memory.h" // Подключаем память

extern "C" void draw_all_ui(); 
extern "C" uint32_t end;    // Эту метку нам автоматически предоставляет linker.ld (конец бинарника ядра)

extern "C" void kmain(uint32_t magic, uint32_t addr) {
    (void)magic;

    // ИСПРАВЛЕНО: Запускаем менеджер памяти. Куча начнется сразу за концом ядра
    init_memory((uint32_t)&end);

    uint32_t* fb = (uint32_t*)(*(uint32_t*)(addr + 88));
    init_graphics(fb); // Инициализация графики (теперь и с бэкбуфером!)

    init_idt();
    init_mouse_fs();     
    init_mouse_driver();

    draw_all_ui();

    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}