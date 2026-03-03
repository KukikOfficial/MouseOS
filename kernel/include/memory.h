#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

void init_memory(uint32_t start_addr);
void* kmalloc(size_t size); // Наш аналог malloc

#endif
