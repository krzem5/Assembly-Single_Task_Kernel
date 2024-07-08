#ifndef _KERNEL_VFS_LOCK_H_
#define _KERNEL_VFS_LOCK_H_ 1
#include <kernel/mp/process_group.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



void vfs_lock_init(vfs_node_t* node);



bool vfs_lock_verify_process_group(vfs_node_t* node,process_group_t* process_group);



bool vfs_lock_verify_process(vfs_node_t* node,process_t* process);



bool vfs_lock_verify_thread(vfs_node_t* node,thread_t* thread);



void vfs_lock_lock(vfs_node_t* node,u32 type,handle_id_t handle);



void vfs_lock_unlock(vfs_node_t* node,u32 type);



#endif
