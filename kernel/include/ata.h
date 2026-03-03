#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// Оборачиваем в extern "C", чтобы C++ не менял имена функций
extern "C" {
    void ata_read_sector(uint32_t lba, uint16_t* buffer);
    void ata_write_sector(uint32_t lba, uint16_t* buffer);
}

#endif
