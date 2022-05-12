#include "kernel.hh"
#include "atomic"
unsigned long ticks;

struct pt_regs * task_pt_regs(struct proc *task) {
	unsigned long p = (unsigned long)task + PROC_SIZE - sizeof(struct pt_regs);
	return (struct pt_regs *)p;
}

void kernel_process(){
	printf("Kernel process started. EL %d\r\n", get_el());
	unsigned long begin = (unsigned long)&user_begin;
	unsigned long end = (unsigned long)&user_end;
	unsigned long process = (unsigned long)&process_main;
	int err = init_user(begin, end - begin, process - begin);
	if (err < 0){
		printf("Error while moving process to user mode\n\r");
	} 
}

int copy_process(unsigned long clone_flags, unsigned long fn, unsigned long arg)
{
	preempt_disable();
	int child_pid = -1;
	for (int pid = 1; pid < NPROC; pid++){
       if (ptable[pid] == nullptr){
		   child_pid = pid;
           break;
        }
    }

	if (child_pid == -1) {
		return -1;
	}

	struct proc *p = (struct proc*) kalloc();
	struct pt_regs *childregs = task_pt_regs(p);

	if (!p) {
		return -1;
	}

	if (clone_flags & PF_KTHREAD) {
		p->regs_.x19 = fn;
		p->regs_.x20 = arg;
	} else {
		struct pt_regs * cur_regs = task_pt_regs(current);
		*cur_regs = *childregs;
		childregs->regs[0] = 0;
		copy_virt_memory(p);
	}
	p->id_ = child_pid;
	p->flags = clone_flags;
	p->priority_ = current->priority_;
	p->pstate_ = proc::ps_runnable;
	p->counter_ = p->priority_;
	p->preempt_count_ = 1; //disable preemtion until schedule_tail

	p->regs_.pc = (unsigned long)ret_from_fork;
	p->regs_.sp = (unsigned long)childregs;
	ptable[child_pid] = p;	
	preempt_enable();
	return child_pid;
}


int init_user(unsigned long start, unsigned long size, unsigned long pc)
{
	struct pt_regs *regs = task_pt_regs(current);
	regs->pstate = PSR_MODE_EL0t;
	regs->pc = pc;
	regs->sp = 2 * PAGE_SIZE;  
	unsigned long code_page = ualloc(current, 0);
	if (code_page == 0)	{
		return -1;
	}
	memcpy(start, code_page, size);
	set_pagetable(current->pagetable_);
	return 0;
}

void kernel_start() {
	init_hardware();

	printf("Total available memory is %d pages\n", PAGING_PAGES);

	int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0);
	if (res < 0) {
		printf("error while starting kernel process\n\r");
		return;
	}

	while (1){
		schedule();
	}	
}

void memshow() {
    static unsigned long last_switch = 0;
    static int showing = 1;

    // switch to a new process every 1 sec
    if (ticks - last_switch >= HZ / 20) {
        showing = (showing + 1) % NPROC;
        last_switch = ticks;
    }

    int search = 0;
    while ((!ptable[showing]
            || !ptable[showing]->pagetable_)
           && search < NPROC) {
        showing = (showing + 1) % NPROC;
        ++search;
    }

    console_memviewer(ptable[showing]);
    if (!ptable[showing]) {
        console_print(10, 425, "[All processes have exited]", 0x0000FF);
    }
}