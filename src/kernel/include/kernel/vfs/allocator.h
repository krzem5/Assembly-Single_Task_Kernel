#ifndef _KERNEL_VFS_ALLOCATOR_H_
#define _KERNEL_VFS_ALLOCATOR_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/vfs/vfs.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define VFS_ALLOCATOR_SIZE_SHIFT 8



typedef u16 vfs_allocator_index_t;



typedef struct _VFS_ALLOCATOR_ENTRY{
	vfs_node_id_t id;
	vfs_node_t* node;
	vfs_allocator_index_t id_at_index;
	vfs_allocator_index_t prev;
	vfs_allocator_index_t next;
} vfs_allocator_entry_t;



typedef struct _VFS_ALLOCATOR{
	lock_t lock;
	u8 vfs_index;
	vfs_allocator_index_t first;
	vfs_allocator_index_t last;
	vfs_node_id_t next_id;
	vfs_allocator_entry_t* data;
	vfs_node_t* root_node;
} vfs_allocator_t;



void vfs_allocator_init(u8 vfs_index,u8 node_size,vfs_allocator_t* out);



vfs_node_t* vfs_allocator_get(vfs_allocator_t* allocator,vfs_node_id_t id,_Bool allocate_if_not_present);



#endif
