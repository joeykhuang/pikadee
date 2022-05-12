#include "u-lib.hh"
#include "printf.hh"
#include "k-alloc.hh"

#define ALLOC_SLOWDOWN 100

extern uint8_t end[];

uint8_t* heap_top;
uint8_t stack_bottom_val;
uint8_t* stack_bottom;
static int rand_seed_set;
static unsigned long rand_seed;

void srand(unsigned seed) {
    rand_seed = ((unsigned long) seed << 32) | seed;
    rand_seed_set = 1;
}

int rand() {
    if (!rand_seed_set) {
        srand(819234718U);
    }
    rand_seed = rand_seed * 6364136223846793005UL + 1;
    return (rand_seed >> 32) & RAND_MAX;
}

int rand(int min, int max) {
    assert(min <= max);
    assert(max - min <= RAND_MAX);

    unsigned long r = rand();
    return min + (r * (max - min + 1)) / ((unsigned long) RAND_MAX + 1);
}
void process_main() {
    // Fork three new copies. (But ignore failures.)
    (void) sys_fork();
    (void) sys_fork();
    heap_top += 2 * PAGE_SIZE;

    int pid = sys_getpid();
    srand(pid);


    while (true) {
        if (rand(0, ALLOC_SLOWDOWN - 1) < pid) {
            sys_page_alloc(heap_top);
            heap_top += PAGE_SIZE;
        }
        sys_yield();
        if (rand() < RAND_MAX / 32) {
            sys_pause(100000000);
        }
    }

    // After running out of memory, do nothing forever
    while (true) {
        sys_yield();
    }
}