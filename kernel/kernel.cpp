#include "include/vesa.h"
#include "include/font.h"
#include "include/mouse.h"
#include "include/idt.h"
#include "include/fs.h" // Добавили заголовок ФС

// Объявляем функцию отрисовки из mouse.cpp, чтобы обновить экран при старте
extern "C" void draw_all_ui(); 

extern "C" void kmain(uint32_t magic, uint32_t addr) {
    (void)magic;
    // Получаем адрес Framebuffer из структуры Multiboot
    uint32_t* fb = (uint32_t*)(*(uint32_t*)(addr + 88));
    init_graphics(fb);

    // Сначала настраиваем прерывания и ФС
    init_idt();
    init_mouse_fs();     // ИСПРАВЛЕНО: Теперь суперблок и таблица файлов читаются с диска!
    init_mouse_driver();

    // Отрисовываем весь графический интерфейс ОС
    draw_all_ui();

    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}