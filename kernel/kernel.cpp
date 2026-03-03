#include "include/vesa.h"
#include "include/font.h"
#include "include/mouse.h"
#include "include/idt.h"

extern "C" void kmain(uint32_t magic, uint32_t addr) {
    (void)magic;
    uint32_t* fb = (uint32_t*)(*(uint32_t*)(addr + 88));
    init_graphics(fb);

    clear_screen(0x1A2B3C); 

    // Начальный UI
    draw_rect(150, 100, 724, 500, 0xCCCCCC); 
    draw_rect(150, 100, 724, 32, 0x0055AA);  
    draw_string(160, 108, "MOUSE OS CORE", 0xFFFFFF);
    draw_rect(0, 730, 1024, 38, 0x222222);
    draw_string(10, 742, "[ START ]", 0xFFFFFF);

    init_idt();
    init_mouse_driver();

    while (1) {
        __asm__ __volatile__ ("hlt");
    }
}
