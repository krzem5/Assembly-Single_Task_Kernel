#ifndef _KERNEL_SHMEM_SHMEM_H_
#define _KERNEL_SHMEM_SHMEM_H_ 1
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/rwlock.h>
#include <kernel/mmap/mmap.h>
#include <kernel/types.h>



#define SHMEM_REGION_FLAG_WRITABLE 1
#define SHMEM_REGION_FLAG_DELETABLE 2
#define SHMEM_REGION_FLAG_DELETED 4
#define SHMEM_REGION_FLAG_ALLOCATED 8

#define SHMEM_BYPASS_FLAG_REGION_FLAGS 1
#define SHMEM_BYPASS_FLAG_ACL 2



typedef struct _SHMEM_PARENT_REGION{
	handle_t handle;
	rwlock_t lock;
	u32 flags;
	void* ptr;
	u64 size;
} shmem_parent_region_t;



typedef struct _SHMEM_REGION{
	handle_t handle;
	rwlock_t lock;
	u32 flags;
	handle_id_t parent_handle;
	mmap_region_t* region;
	u64 size;
} shmem_region_t;



shmem_parent_region_t* shmem_parent_region_create(void* ptr,u64 size,u32 flags);



void shmem_parent_region_delete(shmem_parent_region_t* region,u32 bypass_flags);



shmem_region_t* shmem_region_create(shmem_parent_region_t* parent,u32 flags);



void shmem_region_delete(shmem_region_t* region,u32 bypass_flags);



error_t shmem_region_map(shmem_region_t* region);



error_t shmem_region_unmap(shmem_region_t* region);



#endif
