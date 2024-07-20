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



static omm_allocator_t* KERNEL_INIT_WRITE _shmem_parent_region_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _shmem_region_allocator=NULL;
static handle_id_t KERNEL_INIT_WRITE _shmem_parent_region_handle_type=0;
static handle_id_t KERNEL_INIT_WRITE _shmem_region_handle_type=0;



KERNEL_INIT(){
	_shmem_parent_region_allocator=omm_init("kernel.shmem.region.parent",sizeof(shmem_parent_region_t),8,4);
	rwlock_init(&(_shmem_parent_region_allocator->lock));
	_shmem_region_allocator=omm_init("kernel.shmem.region",sizeof(shmem_region_t),8,4);
	rwlock_init(&(_shmem_region_allocator->lock));
	_shmem_parent_region_handle_type=handle_alloc("kernel.shmem.region.parent",0,NULL);
	_shmem_region_handle_type=handle_alloc("kernel.shmem.region",0,NULL);
}



KERNEL_PUBLIC shmem_parent_region_t* shmem_parent_region_create(void* ptr,u64 size,u32 flags){
	if ((((u64)ptr)|size)&(PAGE_SIZE-1)){
		panic("shmem_parent_region_create: unaligned arguments");
	}
	shmem_parent_region_t* out=omm_alloc(_shmem_parent_region_allocator);
	handle_new(_shmem_parent_region_handle_type,&(out->handle));
	rwlock_init(&(out->lock));
	out->flags=flags;
	out->ptr=ptr;
	out->size=size;
	return out;
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
