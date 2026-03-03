#include "include/memory.h"

uint32_t free_mem_addr;

void init_memory(uint32_t start_addr) {
    // Устанавливаем начало кучи (обычно сразу за концом ядра)
    free_mem_addr = start_addr;
}

void* kmalloc(size_t size) {
    // Выравниваем по 4 байта для скорости
    if (size % 4 != 0) {
        size += (4 - (size % 4));
    }
    
    uint32_t ret = free_mem_addr;
    free_mem_addr += size; // Сдвигаем указатель на размер запроса
    return (void*)ret;
}
