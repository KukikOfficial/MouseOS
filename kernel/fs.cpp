#include "include/fs.h"
#include "include/ata.h"
#include "include/vga.h"

SuperBlock sb;
// Глобальный массив записей о файлах
DiskFileEntry file_table[MAX_DISK_FILES]; 

extern "C" {

bool fs_str_match(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0' && s2[i] != ' ' && s2[i] != '\n') {
        if (s1[i] != s2[i]) return false;
        i++;
    }
    return (s1[i] == '\0');
}

void fs_str_copy(char* dest, const char* src) {
    int i = 0;
    while (src[i] && i < 19) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void init_mouse_fs() {
    ata_read_sector(1, (uint16_t*)&sb);
    
    // ИСПРАВЛЕНО: Проверяем не только магическое число, но и адекватность счетчика файлов
    if (sb.magic != MOUSE_FS_MAGIC || sb.file_count > MAX_DISK_FILES) {
        terminal_writestring("MouseCore: Formatting Disk (Invalid or Empty FS)...\n");
        sb.magic = MOUSE_FS_MAGIC;
        sb.file_count = 0;
        ata_write_sector(1, (uint16_t*)&sb);
        
        for(int i = 0; i < MAX_DISK_FILES; i++) file_table[i].flags = 0;
        ata_write_sector(2, (uint16_t*)file_table);
    } else {
        ata_read_sector(2, (uint16_t*)file_table);
    }
}

uint32_t get_file_count() {
    return sb.file_count;
}

void write_to_disk(const char* name, const char* content) {
    if (sb.file_count >= MAX_DISK_FILES) return;
    uint32_t idx = sb.file_count;
    
    fs_str_copy(file_table[idx].name, name);
    file_table[idx].start_lba = 10 + idx;
    file_table[idx].flags = 1;

    uint16_t buffer[256];
    for(int j = 0; j < 256; j++) buffer[j] = 0;
    char* ptr = (char*)buffer;
    for(int j = 0; content[j] != '\0' && j < 510; j++) ptr[j] = content[j];

    ata_write_sector(file_table[idx].start_lba, buffer);
    sb.file_count++;
    ata_write_sector(1, (uint16_t*)&sb);
    ata_write_sector(2, (uint16_t*)file_table);
}

void read_from_disk(const char* name) {
    for (uint32_t i = 0; i < sb.file_count; i++) {
        if (file_table[i].flags == 1 && fs_str_match(file_table[i].name, name)) {
            uint16_t buffer[256];
            ata_read_sector(file_table[i].start_lba, buffer);
            terminal_writestring((char*)buffer);
            return;
        }
    }
}

void list_files() {
    for (uint32_t i = 0; i < sb.file_count; i++) {
        if (file_table[i].flags == 1) {
            terminal_writestring(file_table[i].name);
            terminal_writestring("\n");
        }
    }
}

} // extern "C"
