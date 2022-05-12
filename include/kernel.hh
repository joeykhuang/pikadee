#ifndef PIKADEE_KERNEL_HH
#define PIKADEE_KERNEL_HH

#include "printf.hh"
#include "armv8.hh"
#include "types.h"
#include "utils.hh"
#include "k-interrupts.hh"
#include "k-cpu.hh"

#define MAX_PROCESS_PAGES	1000
#define PROC_RUNNABLE       1

#define PROC_SIZE			4096
#define PF_KTHREAD			0x00000002	

#define NPROC				16
#define HZ                  100

struct proc;

extern struct proc *current;
extern struct proc *ptable[NPROC];
extern unsigned long user_begin;
extern unsigned long user_end;

struct regs_ {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};



#define FIRST_TASK ptable[0]
#define LAST_TASK ptable[NPROC-1]

void init_hardware();
void process_main();

enum memtype_t {
    mem_nonexistent = 0,
    mem_available = 1,
    mem_kernel = 2,
    mem_reserved = 3,
    mem_console = 4
};
struct pt_regs {
	unsigned long regs[31];
	unsigned long sp;
	unsigned long pc;
	unsigned long pstate;
};

struct user_page {
	unsigned long phys_addr;
	unsigned long virt_addr;
};

struct mm_struct {
	int user_pages_count;
	int kernel_pages_count;

	struct user_page user_pages[MAX_PROCESS_PAGES];
	unsigned long kernel_pages[MAX_PROCESS_PAGES];
};

struct __attribute__((aligned(4096))) proc {
    enum pstate_t {
        ps_blank = 0, ps_runnable = PROC_RUNNABLE, ps_faulted, ps_exiting, ps_exited, ps_blocked,
    };

	struct regs_ regs_;
    pid_t id_ = 0;
    pstate_t pstate_ = ps_blank;       // Process state
    uintptr_t pagetable_;
	long counter_;
	long priority_;
	long preempt_count_;
	unsigned long flags;
	struct mm_struct mm;
};

/*
 * PSR bits
 */
#define PSR_MODE_EL0t	0x00000000
#define PSR_MODE_EL1t	0x00000004
#define PSR_MODE_EL1h	0x00000005
#define PSR_MODE_EL2t	0x00000008
#define PSR_MODE_EL2h	0x00000009
#define PSR_MODE_EL3t	0x0000000c
#define PSR_MODE_EL3h	0x0000000d

int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg);
int init_user(unsigned long start, unsigned long size, unsigned long pc);
extern "C" void ret_from_fork(void);

// Print memory viewer
void console_memviewer(proc* p);
void memshow();

#endif