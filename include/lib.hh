#ifndef PIKADEE_LIB_HH
#define PIKADEE_LIB_HH

#define SYSCALL_WRITE           0
#define SYSCALL_GETPID          1
#define SYSCALL_FORK            2
#define SYSCALL_PAGE_ALLOC      3
#define SYSCALL_EXIT            4
#define SYSCALL_YIELD           5
#define SYSCALL_PRINT           6

#define CONSOLE_COLUMNS     750 
#define CONSOLE_ROWS        450
#define CPOS(row, col)      ((row) * 80 + (col))

#define PAGE_SHIFT	 		    12
#define PAGE_SIZE   			(1 << PAGE_SHIFT)	

#ifndef __ASSEMBLER__
#include "types.h"
// Assertions

// assert(x)
//    If `x == 0`, print a message and fail.
#define assert(x, ...)       do {                                       \
        if (!(x)) {                                                     \
            assert_fail(__FILE__, __LINE__, #x, ## __VA_ARGS__);        \
        }                                                               \
    } while (false)
__attribute__((noinline, cold))
void assert_fail(const char* file, int line, const char* msg,
                 const char* description = nullptr);


// assert_[eq, ne, lt, le, gt, ge](x, y)
//    Like `assert(x OP y)`, but also prints the values of `x` and `y` on
//    failure.
#define assert_op(x, op, y) do {                                        \
        auto __x = (x); auto __y = (y);                                 \
        using __t = typename std::common_type<typeof(__x), typeof(__y)>::type; \
        if (!(__x op __y)) {                                            \
            assert_op_fail<__t>(__FILE__, __LINE__, #x " " #op " " #y,  \
                                __x, #op, __y);                         \
        } } while (0)
#define assert_eq(x, y) assert_op(x, ==, y)
#define assert_ne(x, y) assert_op(x, !=, y)
#define assert_lt(x, y) assert_op(x, <, y)
#define assert_le(x, y) assert_op(x, <=, y)
#define assert_gt(x, y) assert_op(x, >, y)
#define assert_ge(x, y) assert_op(x, >=, y)

template <typename T>
__attribute__((noinline, noreturn, cold))
void assert_op_fail(const char* file, int line, const char* msg,
                    const T& x, const char* op, const T& y) {
    assert_fail(file, line, msg);
}


// assert_memeq(x, y, sz)
//    If `memcmp(x, y, sz) != 0`, print a message and fail.
#define assert_memeq(x, y, sz)    do {                                  \
        auto __x = (x); auto __y = (y); size_t __sz = (sz);             \
        if (memcmp(__x, __y, __sz) != 0) {                              \
            assert_memeq_fail(__FILE__, __LINE__, "memcmp(" #x ", " #y ", " #sz ") == 0", __x, __y, __sz); \
        }                                                               \
    } while (0)
__attribute__((noinline, noreturn, cold))
void assert_memeq_fail(const char* file, int line, const char* msg,
                       const char* x, const char* y, size_t sz);


template <typename T>
inline constexpr T round_down(T x, unsigned m) {
    return x - (x % m);
}

// round_up(x, m)
//    Return the smallest multiple of `m` greater than or equal to `x`.
//    Equivalently, round `x` up to the nearest multiple of `m`.
template <typename T>
inline constexpr T round_up(T x, unsigned m) {
    return round_down(x + m - 1, m);
}

inline constexpr int lsb(int x) {
    return __builtin_ffs(x);
}
inline constexpr int lsb(unsigned x) {
    return __builtin_ffs(x);
}
inline constexpr int lsb(long x) {
    return __builtin_ffsl(x);
}
inline constexpr int lsb(unsigned long x) {
    return __builtin_ffsl(x);
}
inline constexpr int lsb(long long x) {
    return __builtin_ffsll(x);
}
inline constexpr int lsb(unsigned long long x) {
    return __builtin_ffsll(x);
}

#endif
#endif