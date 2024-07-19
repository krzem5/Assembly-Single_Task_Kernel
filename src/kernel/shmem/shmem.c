#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/shmem/shmem.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "shmem"



KERNEL_INIT(){
	return;
}



KERNEL_PUBLIC shmem_parent_region_t* shmem_parent_region_create(void* ptr,u64 size,u32 flags){
	panic("shmem_parent_region_create");
}



KERNEL_PUBLIC void shmem_parent_region_delete(shmem_parent_region_t* region,u32 bypass_flags){
	panic("shmem_parent_region_delete");
}



KERNEL_PUBLIC shmem_region_t* shmem_region_create(shmem_parent_region_t* parent,u32 flags){
	panic("shmem_region_create");
}



KERNEL_PUBLIC void shmem_region_delete(shmem_region_t* region,u32 bypass_flags){
	panic("shmem_region_delete");
}



KERNEL_PUBLIC void shmem_region_map(shmem_region_t* region){
	panic("shmem_region_map");
}
