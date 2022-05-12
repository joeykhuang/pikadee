#ifndef PIKADEE_U_LIB_HH
#define PIKADEE_U_LIB_HH

#include "lib.hh"

extern "C" {
    int sys_fork();
    int sys_getpid();
    int sys_page_alloc(void* addr);
    void sys_exit();
    void sys_pause(int times);
    void sys_yield();
    void sys_write(char* buf, ...);
    void sys_print(int x, int y, char* buf);
    uintptr_t rdrsp();
}

#define RAND_MAX 0x7FFFFFFF

#endif