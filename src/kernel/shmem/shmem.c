#include <kernel/error/error.h>
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



static void _parent_region_handle_destructor(handle_t* handle){
	shmem_parent_region_t* parent=KERNEL_CONTAINEROF(handle,shmem_parent_region_t,handle);
	if ((parent->flags&(SHMEM_REGION_FLAG_DELETED|SHMEM_REGION_FLAG_ALLOCATED))==SHMEM_REGION_FLAG_ALLOCATED){
		WARN("Dealloc memory");
	}
	omm_dealloc(_shmem_parent_region_allocator,parent);
}



static void _region_handle_destructor(handle_t* handle){
	shmem_region_t* region=KERNEL_CONTAINEROF(handle,shmem_region_t,handle);
	if (!(region->flags&SHMEM_REGION_FLAG_DELETED)&&region->region){
		WARN("Dealloc mmap memory");
	}
	omm_dealloc(_shmem_region_allocator,region);
}



KERNEL_INIT(){
	_shmem_parent_region_allocator=omm_init("kernel.shmem.region.parent",sizeof(shmem_parent_region_t),8,4);
	rwlock_init(&(_shmem_parent_region_allocator->lock));
	_shmem_region_allocator=omm_init("kernel.shmem.region",sizeof(shmem_region_t),8,4);
	rwlock_init(&(_shmem_region_allocator->lock));
	_shmem_parent_region_handle_type=handle_alloc("kernel.shmem.region.parent",0,_parent_region_handle_destructor);
	_shmem_region_handle_type=handle_alloc("kernel.shmem.region",0,_region_handle_destructor);
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
	if (!ptr){
		WARN("Alloc memory + SHMEM_REGION_FLAG_ALLOCATED");
	}
	return out;
}



KERNEL_PUBLIC void shmem_parent_region_delete(shmem_parent_region_t* region,u32 bypass_flags){
	rwlock_acquire_write(&(region->lock));
	if (!(region->flags&SHMEM_REGION_FLAG_DELETED)){
		if (region->flags&SHMEM_REGION_FLAG_ALLOCATED){
			WARN("Dealloc memory");
		}
		region->ptr=NULL;
	}
	region->flags|=SHMEM_REGION_FLAG_DELETED;
	rwlock_release_write(&(region->lock));
	handle_release(&(region->handle));
}



KERNEL_PUBLIC shmem_region_t* shmem_region_create(shmem_parent_region_t* parent,u32 flags){
	if (parent->flags&SHMEM_REGION_FLAG_DELETED){
		return NULL;
	}
	handle_acquire(&(parent->handle));
	shmem_region_t* out=omm_alloc(_shmem_region_allocator);
	handle_new(_shmem_region_handle_type,&(out->handle));
	rwlock_init(&(out->lock));
	out->flags=flags&parent->flags&(SHMEM_REGION_FLAG_WRITABLE|SHMEM_REGION_FLAG_DELETABLE);
	out->parent_handle=parent->handle.rb_node.key;
	out->region=NULL;
	out->size=parent->size;
	return out;
}



KERNEL_PUBLIC void shmem_region_delete(shmem_region_t* region,u32 bypass_flags){
	rwlock_acquire_write(&(region->lock));
	if (!(region->flags&SHMEM_REGION_FLAG_DELETED)&&region->region){
		WARN("Dealloc mmap region");
		region->region=NULL;
	}
	region->flags|=SHMEM_REGION_FLAG_DELETED;
	rwlock_release_write(&(region->lock));
	handle_release(&(region->handle));
}



KERNEL_PUBLIC error_t shmem_region_map(shmem_region_t* region){
	rwlock_acquire_write(&(region->lock));
	if (region->flags&SHMEM_REGION_FLAG_DELETED){
		rwlock_release_write(&(region->lock));
		return ERROR_INVALID_HANDLE;
	}
	if (region->region){
		rwlock_release_write(&(region->lock));
		return ERROR_ALREADY_MAPPED;
	}
	WARN("Map region");
	rwlock_release_write(&(region->lock));
	return 0;
}



KERNEL_PUBLIC error_t shmem_region_unmap(shmem_region_t* region){
	rwlock_acquire_write(&(region->lock));
	if (region->flags&SHMEM_REGION_FLAG_DELETED){
		rwlock_release_write(&(region->lock));
		return ERROR_INVALID_HANDLE;
	}
	if (!region->region){
		rwlock_release_write(&(region->lock));
		return ERROR_UNSUPPORTED_OPERATION;
	}
	WARN("Unmap region");
	rwlock_release_write(&(region->lock));
	return ERROR_OK;
}



error_t syscall_shmem_create_parent(u64 size){
	WARN("syscall_shmem_create_parent");
	return 0;
}



error_t syscall_shmem_delete_parent(handle_id_t shmem_parent_region){
	WARN("syscall_shmem_delete_parent");
	return 0;
}



error_t syscall_shmem_create(handle_id_t shmem_parent_region){
	WARN("syscall_shmem_create");
	return 0;
}



error_t syscall_shmem_delete(handle_id_t shmem_parent_region){
	WARN("syscall_shmem_delete");
	return 0;
}



error_t syscall_shmem_map(handle_id_t shmem_region){
	WARN("syscall_shmem_map");
	return 0;
}



error_t syscall_shmem_unmap(handle_id_t shmem_region){
	WARN("syscall_shmem_unmap");
	return 0;
}
