#include "kernel.hh"
#include "printf.hh"

// k-memviewer.cc
//
//    The `memusage` class tracks memory usage by walking page tables,
//    looks for errors, and prints the memory map to the console.

extern unsigned char* console;

class memusage {
  public:
    // tracks physical addresses in the range [0, maxpa)
    static constexpr uintptr_t maxpa = 1024 * PAGE_SIZE;
    // shows physical addresses in the range [0, max_view_pa)
    static constexpr uintptr_t max_view_pa = 512 * PAGE_SIZE;
    // shows virtual addresses in the range [0, max_view_va)
    static constexpr uintptr_t max_view_va = 768 * PAGE_SIZE;

    memusage()
        : v_(nullptr) {
    }

    // Flag bits for memory types:
    static constexpr unsigned f_kernel = 1;     // kernel-restricted
    static constexpr unsigned f_user = 2;       // user-accessible
    // `f_process(pid)` is for memory associated with process `pid`
    static constexpr unsigned f_process(int pid) {
        if (pid >= 30) {
            return 2U << 31;
        } else if (pid >= 1) {
            return 2U << pid;
        } else {
            return 0;
        }
    }
    // Pages such as process page tables and `struct proc` are counted
    // both as kernel-only and process-associated.


    // Refresh the memory map from current state
    void refresh();

    // Return the symbol (character & color) associated with `pa`
    char symbol_at(uintptr_t pa) const;

  private:
    unsigned* v_;

    // add `flags` to the page containing `pa`
    // This is safe to call even if `pa >= maxpa`.
    void mark(uintptr_t pa, unsigned flags) {
        if (pa < maxpa) {
            v_[pa / PAGE_SIZE] |= flags;
        }
    }
    // return one of the processes set in a mark
    static int marked_pid(unsigned v) {
        return lsb(v >> 2);
    }
    // print an error about a page table
    void page_error(uintptr_t pa, const char* desc, int pid) const;
};


// memusage::refresh()
//    Calculate the current physical usage map, using the current process
//    table.

void memusage::refresh() {
    if (!v_) {
        v_ = reinterpret_cast<unsigned*>(kalloc());
        assert(v_ != nullptr);
    }

    memzero((uintptr_t) v_, (maxpa / PAGE_SIZE) * sizeof(*v_));

    mark(ka2pa(v_) - maxpa, f_kernel); // mark v_ as kernel accessible only

    // mark pages accessible from process page tables
    for (int pid = 1; pid < NPROC; ++pid) {
        proc* p = ptable[pid];
        if (p) {
            mark(ka2pa(p) - maxpa, f_kernel | f_process(pid));

            if (p->pagetable_) {
                for (int i = 0; i < p->mm.kernel_pages_count; i++) {
                    mark(p->mm.kernel_pages[i] - maxpa, f_kernel | f_process(pid));
                }
                mark(p->pagetable_ - maxpa, f_kernel | f_process(pid));
                for (int i = 0; i < p->mm.user_pages_count; i++) {
                    mark(p->mm.user_pages[i].phys_addr - maxpa, f_user | f_process(pid));
                }
            }
        }
    }
}

void memusage::page_error(uintptr_t pa, const char* desc, int pid) const {
    const char* fmt = pid >= 0
        ? "PAGE TABLE ERROR: %lx: %s (pid %d)\n"
        : "PAGE TABLE ERROR: %lx: %s\n";
    printf((char*) fmt, pa, desc, pid);
}

char memusage::symbol_at(uintptr_t pa) const {
    auto v = v_[pa / PAGE_SIZE];
    if (pa == 0) {
        return 'R';
    }
    if (v == 0) {
        return '.';
    } else if (v == f_kernel) {
        return 'K';
    } else if ((v & f_kernel) && (v & f_user)) {
        // kernel-restricted + user-accessible = error
        page_error(pa, "sharing error, kernel-restricted + user-accessible\n",
                   marked_pid(v));
        return '*';
    } else {
        char ch;
        // find lowest process involved with this page
        pid_t pid = marked_pid(v);
        // foreground color is that associated with `pid`
        if (v & f_kernel) {
            // kernel page: dark red background
            ch = 'K';
            return ch;
        }
        if (v > (f_process(pid) | f_kernel | f_user)) {
            // shared page
            ch = 'S';
        } else {
            // non-shared page
            static const char names[] = "K123456789ABCDEFGHIJKLMNOPQRST??";
            ch = names[pid];
        }
        return ch;
    }
}


static void console_memviewer_virtual(memusage& mu, proc* vmp) {
    char title[40];
    tfp_sprintf(title, "VIRTUAL ADDRESS SPACE FOR %d\n", vmp->id_);
    console_print(280, 200, title);

    for (int vn = 0; vn * PAGE_SIZE < (int) memusage::max_view_va; ++vn){
        if (vn % 64 == 0) {
            char c[10];
            tfp_sprintf(c, "0x%06X\n", vn * PAGE_SIZE);
            console_print(15, 220 + vn/4, c);
        }

        if (vn < vmp->mm.user_pages_count) {
            char symbol[1];
            symbol[0] = '0' + vmp->id_;
            static const uint32_t colors[] = { 0xFFFFFF, 0x1DC217, 0x4050FF, 0xFBCDFF, 0x99FFFF};
            uint32_t ch = colors[vmp->id_ % 5];
            console_print(100 + 10 * (vn%64), 220 + 16*(vn / 64), symbol, ch, 0x000000);
        } else {
            console_print(100 + 10 * (vn%64), 220 + 16*(vn / 64), ".");
        }
    }
}


static memusage mu;

void console_memviewer(proc* vmp) {
    // track physical memory
    mu.refresh();
    // print physical memory
    console_print(300, 10, "PHYSICAL MEMORY");

    for (int pn = 0; pn * PAGE_SIZE < (int) memusage::max_view_pa; ++pn) {
        if (pn % 64 == 0) {
            char c[10];
            tfp_sprintf(c, "0x%06X\n", pn * PAGE_SIZE);
            console_print(15, 30 + pn/4, c);
        }
        char symbol = mu.symbol_at(pn * PAGE_SIZE);
        // printf("printing symbol %c\n", symbol);
        pause();
        if (symbol == 'K') {
            console_print(100 + 10 * (pn%64), 30 + 16*(pn / 64), "K", 0xFFFFFF, 0x0000FF);
        } else if (symbol == 'R') {
            console_print(100 + 10 * (pn%64), 30 + 16*(pn / 64), "R", 0xFFFFFF, 0x0000FF);
        } else if (symbol >= '1' && symbol <= '9') {
            char pidtoprint[2]; // weird mechanic with console_print
            pid_t pid = symbol - '0';
            pidtoprint[0] = pid + '0';
            pidtoprint[1] = '\0'; // need to end with null for print to work properly
            static const uint32_t colors[] = { 0xFFFFFF, 0x1DC217, 0x4050FF, 0xFBCDFF, 0x99FFFF};
            uint32_t ch = colors[pid % 5];
            console_print(100 + 10 * (pn%64), 30 + 16*(pn / 64), pidtoprint, ch, 0x000000);
        } else if (symbol == '.') {
            console_print(100 + 10 * (pn%64), 30 + 16*(pn / 64), ".");
        }
    }

    // print virtual memory
    if (vmp) {
        if (vmp->pagetable_) {
            console_memviewer_virtual(mu, vmp);
        }
    }
}