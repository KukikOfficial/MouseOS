#include "include/process.h"

extern "C" {

Process process_table[MAX_PROCESSES];
static uint32_t next_pid = 0;

static void proc_str_copy(char* dest, const char* src) {
    int i = 0;
    while (src[i] && i < 19) { dest[i] = src[i]; i++; }
    dest[i] = '\0';
}

static bool proc_str_match(const char* s1, const char* s2) {
    int i = 0;
    while (s1[i] != '\0' && s2[i] != '\0') {
        if (s1[i] != s2[i]) return false;
        i++;
    }
    return (s1[i] == '\0' && s2[i] == '\0');
}

void init_process_system() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROCESS_STOPPED;
    }
    // Запускаем базовые системные процессы нашей ОС
    create_process("IDLE");
    create_process("SYSTEM_UI");
}

int create_process(const char* name) {
    // Проверим, не запущен ли уже такой процесс
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_STOPPED && proc_str_match(process_table[i].name, name)) {
            return process_table[i].pid;
        }
    }
    // Ищем свободный слот
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_STOPPED) {
            process_table[i].pid = next_pid++;
            proc_str_copy(process_table[i].name, name);
            process_table[i].state = PROCESS_READY;
            return process_table[i].pid;
        }
    }
    return -1;
}

void terminate_process_by_name(const char* name) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != PROCESS_STOPPED && proc_str_match(process_table[i].name, name)) {
            process_table[i].state = PROCESS_STOPPED;
            return;
        }
    }
}

} // extern "C"