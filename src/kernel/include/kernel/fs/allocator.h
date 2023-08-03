#ifndef _KERNEL_FS_ALLOCATOR_H_
#define _KERNEL_FS_ALLOCATOR_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define FS_ALLOCATOR_SIZE_SHIFT 8



typedef u16 fs_allocator_index_t;



typedef struct _FS_ALLOCATOR_ENTRY{
	fs_node_id_t id;
	fs_node_t* node;
	fs_allocator_index_t id_at_index;
	fs_allocator_index_t prev;
	fs_allocator_index_t next;
} fs_allocator_entry_t;



typedef struct _FS_ALLOCATOR{
	lock_t lock;
	u8 fs_index;
	fs_allocator_index_t first;
	fs_allocator_index_t last;
	fs_node_id_t next_id;
	fs_allocator_entry_t* data;
	fs_node_t* root_node;
} fs_allocator_t;



void fs_allocator_init(u8 fs_index,u8 node_size,fs_allocator_t* out);



fs_node_t* fs_allocator_get(fs_allocator_t* allocator,fs_node_id_t id,_Bool allocate_if_not_present);



#endif
