#ifndef PIKADEE_K_CPU_HH
#define PIKADEE_K_CPU_HH

#define THREAD_REG_OFFSET		0 		// offset of regs_ in task_struct 

#define __NR_syscalls	        7

#ifndef __ASSEMBLER__
#include "kernel.hh"

extern void sched_init(void);
extern void schedule(void);
extern void timer_tick(void);
extern void preempt_disable(void);
extern void preempt_enable(void);
extern void switch_to(struct proc* next);
extern void exit_process(void);

extern "C" void cpu_switch_to(struct proc* prev, struct proc* next);

#define INIT_TASK \
/*regs_*/ {{ 0,0,0,0,0,0,0,0,0,0,0,0,0}, \
/* state etc */	0, proc::ps_blank, 0, 0, 15, 0, PF_KTHREAD, \
/* mm */ { 0, 0, {{0}}, 0, {0}} \
}
#endif
#endif
