#include "k-cpu.hh"
#include "k-interrupts.hh"
#include "printf.hh"
#include "utils.hh"
#include "k-alloc.hh"
#include "kernel.hh"

static struct proc init_task = INIT_TASK;
struct proc *current = &(init_task);
struct proc *ptable[NPROC] = {&(init_task), };
extern unsigned long ticks;

void preempt_disable(void)
{
	current->preempt_count_++;
}

void preempt_enable(void)
{
	current->preempt_count_--;
}


void _schedule(void)
{
	preempt_disable();
	int next,c;
	struct proc * p;
	while (1) {
		c = -1;
		next = 0;
		for (int i = 0; i < NPROC; i++){
			p = ptable[i];
			if (p && p->pstate_ == proc::ps_runnable && p->counter_ > c) {
				c = p->counter_;
				next = i;
			}
		}
		if (c) {
			break;
		}
		for (int i = 0; i < NPROC; i++) {
			p = ptable[i];
			if (p) {
				p->counter_ = (p->counter_ >> 1) + p->priority_;
			}
		}
	}
	switch_to(ptable[next]);
	preempt_enable();
}

void schedule(void)
{
	current->counter_ = 0;
	_schedule();
}


void switch_to(struct proc * next) 
{
	if (current == next) 
		return;
	struct proc * prev = current;
	current = next;
	set_pagetable(prev->pagetable_);
	cpu_switch_to(prev, next);
}

void schedule_tail(void) {
	preempt_enable();
}


void timer_tick()
{
	--current->counter_;
	ticks++;
	memshow();
	if (current->counter_>0 || current->preempt_count_ >0) {
		return;
	}
	current->counter_=0;
	enable_irq();
	_schedule();
	disable_irq();
}

void exit_process(){
	preempt_disable();
	for (int i = 0; i < NPROC; i++){
		if (ptable[i] == current) {
			ptable[i]->pstate_ = proc::ps_exiting;
			break;
		}
	}
	preempt_enable();
	schedule();
}
