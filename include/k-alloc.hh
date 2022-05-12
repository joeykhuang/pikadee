#ifndef	PIKADEE_K_ALLOC_HH
#define	PIKADEE_K_ALLOC_HH

#include "rpi3peripherals.hh"
#include "lib.hh"

#define PHYS_MEMORY_SIZE 		0x40000000	

#define PAGE_MASK			    0xFFFFFFFFFFFFF000UL
#define KTEXT_BASE              0xFFFFFFFF80000000UL
#define TABLE_SHIFT 			9
#define SECTION_SHIFT			(PAGE_SHIFT + TABLE_SHIFT)

#define SECTION_SIZE			(1 << SECTION_SHIFT)	

#define LOW_MEMORY              (2 * SECTION_SIZE)
#define HIGH_MEMORY             DEVICE_BASE

#define PAGING_MEMORY 			(HIGH_MEMORY - LOW_MEMORY)
#define PAGING_PAGES 			(PAGING_MEMORY/PAGE_SIZE)

#define PTRS_PER_TABLE			(1 << TABLE_SHIFT)

#define PGD_SHIFT			    PAGE_SHIFT + 3*TABLE_SHIFT
#define PUD_SHIFT			    PAGE_SHIFT + 2*TABLE_SHIFT
#define PMD_SHIFT			    PAGE_SHIFT + TABLE_SHIFT
#define PG_DIR_SIZE			    (3 * PAGE_SIZE)

#ifndef __ASSEMBLER__

#include "k-cpu.hh"

unsigned long get_free_page();
void map_page(struct proc *task, unsigned long va, unsigned long page);

extern "C" void memzero(unsigned long src, unsigned long n);
extern "C" void memcpy(unsigned long src, unsigned long dst, unsigned long n);

int copy_virt_memory(struct proc *dst); 
void* kalloc(); 
void kfree(void* p);
unsigned long ualloc(struct proc *task, unsigned long va); 

extern unsigned long pg_dir;

inline uint64_t pa2ktext(uint64_t pa) {
    assert(pa < -KTEXT_BASE);
    return pa + KTEXT_BASE;
}

template <typename T>
inline T pa2ktext(uint64_t pa) {
    return reinterpret_cast<T>(pa2ktext(pa));
}

inline uint64_t ktext2pa(uint64_t ka) {
    assert(ka >= KTEXT_BASE);
    return ka - KTEXT_BASE;
}

template <typename T>
inline uint64_t ktext2pa(T* ptr) {
    return ktext2pa(reinterpret_cast<uint64_t>(ptr));
}


inline uint64_t pa2ka(uint64_t pa) {
    assert(pa < -VA_START);
    return pa + VA_START;
}

template <typename T>
inline T pa2kptr(uint64_t pa) {
    return reinterpret_cast<T>(pa2ka(pa));
}

inline uint64_t ka2pa(uint64_t ka) {
    // assert(ka >= VA_START && ka < KTEXT_BASE);
    return ka - VA_START;
}

template <typename T>
inline uint64_t ka2pa(T* ptr) {
    return ka2pa(reinterpret_cast<uint64_t>(ptr));
}

inline uint64_t kptr2pa(uint64_t kptr) {
    assert(kptr >= VA_START);
    return kptr - (kptr >= KTEXT_BASE ? KTEXT_BASE : VA_START);
}

template <typename T>
inline uint64_t kptr2pa(T* ptr) {
    return kptr2pa(reinterpret_cast<uint64_t>(ptr));
}

#endif

#endif  /*_MM_H */
