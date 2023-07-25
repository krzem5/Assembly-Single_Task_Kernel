#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



void mmap_init(void){
	LOG("Initializing user mmap...");
}



void mmap_set_range(u64 from,u64 to){
	LOG("Resetting user mmap range to %p - %p...",from,to);
}



u64 mmap_alloc(u64 length){
	WARN("Unimplemented: mmap_alloc");
	return 0;
}



_Bool mmap_dealloc(u64 address,u64 length){
	WARN("Unimplemented: mmap_dealloc");
	return 0;
}
