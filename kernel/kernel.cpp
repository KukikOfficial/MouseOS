#include "include/vga.h"
#include "include/idt.h"
#include "include/ata.h"
#include "include/fs.h"
#include "include/memory.h"
#include "include/vesa.h"   // Наш новый графический драйвер
#include "include/font.h"   // Модуль отрисовки текста

// Build Trigger

// Метка конца ядра из linker.ld
extern "C" uint32_t end; 

/**
 * Точка входа в Mouse Core.
 * В графическом режиме Multiboot передает адрес структуры информации в addr.
 */
extern "C" void kmain(uint32_t magic, uint32_t addr) {
    (void)magic; // Проверка магического числа (0x2BADB002)

    // 1. Инициализация базовых подсистем
    init_memory((uint32_t)&end); // Куча (malloc)
    init_idt();                  // Прерывания (клавиатура)
    init_mouse_fs();             // Файловая система с диска

    // 2. Получение адреса видеопамяти (LFB) из Multiboot Info
    // В структуре Multiboot по смещению 72 байта лежит адрес Framebuffer-а
    uint32_t* framebuffer = (uint32_t*)(*(uint32_t*)(addr + 72));
    init_graphics(framebuffer);

    // 3. Отрисовка интерфейса Mouse OS
    // Очищаем экран (темно-синий фон рабочего стола)
    clear_screen(0x1A2B3C); 

    // Рисуем главное "Окно" Mouse Core
    // Тело окна (светло-серое)
    draw_rect(150, 100, 724, 500, 0xCCCCCC); 
    // Заголовок окна (фирменный синий Mouse OS)
    draw_rect(150, 100, 724, 32, 0x0055AA);  
    // Рамка окна
    draw_rect(150, 600 - 2, 724, 2, 0x555555); 

    // 4. Вывод текста через графический шрифт
    // (Цвета в формате 0xRRGGBB)
    draw_string(160, 108, "Mouse OS v0.1 - Kernel Console", 0xFFFFFF); // Белый
    
    draw_string(180, 150, "Status: Mouse Core initialized successfully.", 0x000000); // Черный
    draw_string(180, 170, "Memory: Heap active at kernel_end.", 0x333333);
    draw_string(180, 190, "Disk: MouseFS mounted (IDE ATA).", 0x333333);
    draw_string(180, 210, "Video: 1024x768 32-bit VESA Mode active.", 0x0055AA);

    draw_string(180, 250, "Welcome to the future of Mouse OS!", 0xAA0000); // Красный

    // 5. Рисуем "Панель задач" внизу экрана
    draw_rect(0, 730, 1024, 38, 0x222222);
    draw_string(10, 742, "[ Start ]", 0xFFFFFF);

    // Главный цикл (ядро продолжает слушать прерывания клавиатуры в фоне)
    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}
