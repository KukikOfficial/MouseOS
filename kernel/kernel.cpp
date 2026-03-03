#include "include/vga.h"
#include "include/idt.h"
#include "include/ata.h"
#include "include/fs.h"
#include "include/memory.h"
#include "include/vesa.h"
#include "include/font.h"
#include "include/mouse.h" // Не забудь создать mouse.h с init_mouse_driver()

extern "C" uint32_t end;

extern "C" void kmain(uint32_t magic, uint32_t addr) {
    (void)magic;

    // 1. Базовая инициализация
    init_memory((uint32_t)&end);
    init_idt(); 
    init_mouse_fs();

    // 2. ИСПРАВЛЕНИЕ БАГА ГРАФИКИ:
    // В Multiboot Info адрес LFB обычно лежит по смещению 88 байт.
    // Мы читаем указатель на структуру, а из неё - физический адрес.
    uint32_t* fb_ptr = (uint32_t*)(*(uint32_t*)(addr + 88));

    if (fb_ptr != 0) {
        init_graphics(fb_ptr);
        
        // Рисуем фон рабочего стола Mouse OS
        clear_screen(0x1A2B3C); 

        // Тестовое окно
        draw_rect(100, 100, 400, 250, 0xCCCCCC); // Тело
        draw_rect(100, 100, 400, 30, 0x0055AA);  // Заголовок
        
        // Текст (используем наш switch-шрифт)
        draw_string(110, 110, "MOUSE OS CORE", 0xFFFFFF);
        draw_string(120, 150, "STATUS: GRAPHICS ACTIVE", 0x000000);
        draw_string(120, 170, "MOUSE DRIVER: LOADING...", 0x333333);
    }

    // 3. Запуск мыши
    init_mouse_driver();

    // Главный цикл
    while (1) {
        // Мышь будет обновляться через прерывания IRQ12
        __asm__ __volatile__ ("hlt");
    }
}
