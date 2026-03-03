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
    uint32_t* fb = (uint32_t*)(*(uint32_t*)(addr + 88));
    init_graphics(fb);

    // Фон Mouse OS
    clear_screen(0x1A2B3C); 

    // Стартовый интерфейс (должен совпадать с redraw_interface_at)
    draw_rect(150, 100, 724, 500, 0xCCCCCC); 
    draw_rect(150, 100, 724, 32, 0x0055AA);  
    draw_string(160, 108, "MOUSE OS CORE", 0xFFFFFF);

    // Панель задач
    draw_rect(0, 730, 1024, 38, 0x222222);
    draw_string(10, 742, "[ START ]", 0xFFFFFF);

    init_idt();
    init_mouse_driver();

    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}
