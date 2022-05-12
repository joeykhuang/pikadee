#include "lib.hh"
.section ".text.user"

.globl sys_pause 
sys_pause:
	subs x0, x0, #1
	bne sys_pause
	ret

.globl sys_write
sys_write:
	mov w8, #SYSCALL_WRITE	
	svc #0
	ret

.globl sys_exit
sys_exit:
	mov w8, #SYSCALL_EXIT
	svc #0
	ret

.globl sys_fork
sys_fork:
	mov w8, #SYSCALL_FORK
	svc #0
	ret

.globl sys_getpid
sys_getpid:
	mov w8, #SYSCALL_GETPID
	svc #0
	ret

.globl sys_yield
sys_yield:
	mov w8, #SYSCALL_YIELD
	svc #0
	ret

.globl sys_page_alloc
sys_page_alloc:
	mov w8, #SYSCALL_PAGE_ALLOC
	svc #0
	ret

.globl sys_print
sys_print:
	mov w8, #SYSCALL_PRINT
	svc #0
	ret