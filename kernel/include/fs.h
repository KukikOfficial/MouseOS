#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stddef.h>

#define MOUSE_FS_MAGIC 0x53554F4D 
#define MAX_DISK_FILES 16         

struct DiskFileEntry {
    char name[20];       // Имя файла (массив символов)
    uint32_t start_lba;  
    uint32_t size_bytes; 
    uint32_t flags;      
};

struct SuperBlock {
    uint32_t magic;      
    uint32_t file_count; 
    uint8_t unused[504]; // Дополняем до 512 байт (сектор)
};

extern "C" {
    void init_mouse_fs();
    void list_files();
    void write_to_disk(const char* name, const char* content);
    void read_from_disk(const char* name);
}

#endif
