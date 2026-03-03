#include "include/ata.h"
#include "include/io.h"

// Ожидание готовности диска
static void ata_wait_bsy() { while (inb(0x1F7) & 0x80); }
static void ata_wait_drq() { while (!(inb(0x1F7) & 0x08)); }

extern "C" void ata_read_sector(uint32_t lba, uint16_t* buffer) {
    ata_wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20); // Команда READ

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(0x1F0);
    }
}

extern "C" void ata_write_sector(uint32_t lba, uint16_t* buffer) {
    ata_wait_bsy();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30); // Команда WRITE

    ata_wait_bsy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buffer[i]);
    }
    
    outb(0x1F7, 0xE7); // Cache Flush
    ata_wait_bsy();
}
