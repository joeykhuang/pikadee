#ifndef PIKADEE_K_LOCK_HH
#define PIKADEE_K_LOCK_HH
#include <atomic>
#include <utility>
#include "armv8.hh"
#include "lib.hh"
#include "k-list.hh"


struct spinlock {
    spinlock() {
        f_.f.clear();
    }

    void lock() {
        while (f_.f.test_and_set()) {
            pause();
        }
    }
    void unlock() {
        f_.f.clear();
    }

    void clear() {
        f_.f.clear();
    }

    bool is_locked() const {
        static_assert(sizeof(f_) == 1, "expect atomic_flag to occupy 1 byte");
        return f_.alias.load(std::memory_order_relaxed) != 0;
    }

private:
    union {
        std::atomic_flag f;
        std::atomic<unsigned char> alias;
    } f_;
};


struct spinlock_guard {
    explicit spinlock_guard(spinlock& lock)
        : lock_(lock), locked_(true) {
    }
    ~spinlock_guard() {
        lock_.unlock();
    }
    NO_COPY_OR_ASSIGN(spinlock_guard);

    void unlock() {
        assert(locked_);
        locked_ = false;
    }
    void lock() {
        assert(!locked_);
        locked_ = true;
    }

    constexpr bool is_locked() {
        return locked_;
    }


    spinlock& lock_;
    bool locked_;
};

#endif
