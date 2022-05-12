#ifndef	PIKADEE_UTILS_HH
#define	PIKADEE_UTILS_HH

#include "armv8.hh"

extern "C" {
    void delay ( unsigned long);
    void put32 ( unsigned long, unsigned int );
    unsigned int get32 ( unsigned long );
    unsigned long get_el ( void );
    void set_pagetable(uintptr_t pagetable);
    uintptr_t get_pagetable();
};

void init_uart ( void );
char uart_recv ( void );
void uart_send ( char c );
void putc ( void* p, char c );
void init_console ( void );

#endif  /*_UTILS_H */
