#include "k-alloc.hh"
#include "armv8.hh"

static unsigned short mem_map [ PAGING_PAGES ] = {0,};

void* kalloc() {
	unsigned long page = get_free_page();
	if (page == 0) {
		return 0;
	}
	return (void*) (page + VA_START);
}

unsigned long ualloc(struct proc *p, unsigned long va) {
	unsigned long page = get_free_page();
	if (page == 0) {
		printf("page is 0 for address %X\n", va);
		return -1;
	}
	map_page(p, va, page);
	return page + VA_START;
}

unsigned long get_free_page()
{
	for (int i = 0; i < PAGING_PAGES; i++){
		if (mem_map[i] == 0){
			mem_map[i] = 1;
			unsigned long page = LOW_MEMORY + i*PAGE_SIZE;
			memzero(page + VA_START, PAGE_SIZE);
			return page;
		}
	}
	return 0;
}

void kfree(void* p){
	mem_map[((unsigned long) p - LOW_MEMORY) / PAGE_SIZE] = 0;
}

void map_table_entry(unsigned long *pte, unsigned long va, unsigned long pa) {
	unsigned long index = va >> PAGE_SHIFT;
	index = index & (PTRS_PER_TABLE - 1);
	unsigned long entry = pa | MMU_PTE_FLAGS; 
	pte[index] = entry;
}

unsigned long map_table(unsigned long *table, unsigned long shift, unsigned long va, int* new_table) {
	unsigned long index = va >> shift;
	index = index & (PTRS_PER_TABLE - 1);
	if (!table[index]){
		*new_table = 1;
		unsigned long next_level_table = get_free_page();
		unsigned long entry = next_level_table | MM_TYPE_PAGE_TABLE;
		table[index] = entry;
		return next_level_table;
	} else {
		*new_table = 0;
	}
	return table[index] & PAGE_MASK;
}

// map page `page` to `va` for process p
void map_page(struct proc *p, unsigned long va, unsigned long page){
	unsigned long pgd;
	if (!p->pagetable_) {
		pgd = get_free_page();
		p->pagetable_ = pgd;
		p->mm.kernel_pages[p->mm.kernel_pages_count++] = pgd;
	} else {
		pgd = p->pagetable_;
	}
	int new_table;
	unsigned long pud = map_table((unsigned long *)(pgd + VA_START), PGD_SHIFT, va, &new_table);
	if (new_table) {
		p->mm.kernel_pages[p->mm.kernel_pages_count++] = pud;
	}
	unsigned long pmd = map_table((unsigned long *)(pud + VA_START) , PUD_SHIFT, va, &new_table);
	if (new_table) {
		p->mm.kernel_pages[p->mm.kernel_pages_count++] = pmd;
	}
	unsigned long pte = map_table((unsigned long *)(pmd + VA_START), PMD_SHIFT, va, &new_table);
	if (new_table) {
		p->mm.kernel_pages[p->mm.kernel_pages_count++] = pte;
	}
	map_table_entry((unsigned long *)(pte + VA_START), va, page);
	struct user_page user_p = {page, va};
	p->mm.user_pages[p->mm.user_pages_count++] = user_p;
}

int copy_virt_memory(struct proc *dst) {
	struct proc* src = current;
	for (int i = 0; i < src->mm.user_pages_count; i++) {
		unsigned long kernel_va = ualloc(dst, src->mm.user_pages[i].virt_addr);
		if( kernel_va == 0) {
			return -1;
		}
		memcpy(src->mm.user_pages[i].virt_addr, kernel_va, PAGE_SIZE);
	}
	return 0;
}

static int ind = 1;

int mem_abort(unsigned long addr, unsigned long esr) {
	unsigned long dfs = (esr & 0b111111);
	if ((dfs & 0b111100) == 0b100) {
		unsigned long page = get_free_page();
		if (page == 0) {
			return -1;
		}
		map_page(current, addr & PAGE_MASK, page);
		ind++;
		if (ind > 2){
			return -1;
		}
		return 0;
	}
	return -1;
}
