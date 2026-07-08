#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define MAX_PROCESSES 8

enum ProcessState {
    PROCESS_STOPPED = 0,
    PROCESS_READY = 1,
    PROCESS_RUNNING = 2
};

struct Process {
    uint32_t pid;
    char name[20];
    ProcessState state;
};

extern "C" {
    void init_process_system();
    int create_process(const char* name);
    void terminate_process_by_name(const char* name);
}

#endif