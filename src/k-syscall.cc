#include "printf.hh"
#include "utils.hh"
#include "k-cpu.hh"
#include "k-alloc.hh"



int syscall_write(char* fmt, ...){
    va_list va;
    va_start(va, fmt);
    printf(fmt, va);
    va_end(va);
    return 0;
}

int syscall_fork(){
    return copy_process(0, 0, 0);
}

int syscall_getpid() {
    return current->id_;
}

int syscall_exit(){
    exit_process();
    return 0;
}

int syscall_yield(){
    // printf("yielding!\n");
    schedule();
    return 0;
}

int syscall_page_alloc(void* addr) {
    return ualloc(current, (uintptr_t) addr);
}

int syscall_print(int x, int y, char* buf) {
    console_print(x, y, buf);
    return 0;
}

int (*syscall_table[])() = {reinterpret_cast<int (*)()>(syscall_write), 
                            syscall_getpid, 
                            syscall_fork, 
                            reinterpret_cast<int (*)()>(syscall_page_alloc), 
                            syscall_exit, 
                            syscall_yield,
                            reinterpret_cast<int (*)()>(syscall_print)};