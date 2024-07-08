#include <kernel/lock/rwlock.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



KERNEL_PUBLIC void vfs_lock_init(vfs_node_t* node){
	rwlock_init(&(node->io_lock.lock));
	node->io_lock.type=VFS_NODE_LOCK_TYPE_NONE;
	node->io_lock.handle=0;
}



KERNEL_PUBLIC bool vfs_lock_verify_process_group(vfs_node_t* node,process_group_t* process_group){
	rwlock_acquire_read(&(node->io_lock.lock));
	bool out=(node->io_lock.type==VFS_NODE_LOCK_TYPE_NONE||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS_GROUP&&node->io_lock.handle==process_group->handle.rb_node.key));
	rwlock_release_read(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_verify_process(vfs_node_t* node,process_t* process){
	rwlock_acquire_read(&(node->io_lock.lock));
	bool out=(node->io_lock.type==VFS_NODE_LOCK_TYPE_NONE||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS_GROUP&&node->io_lock.handle==process->process_group->handle.rb_node.key)||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS&&node->io_lock.handle==process->handle.rb_node.key));
	rwlock_release_read(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_verify_thread(vfs_node_t* node,thread_t* thread){
	rwlock_acquire_read(&(node->io_lock.lock));
	bool out=(node->io_lock.type==VFS_NODE_LOCK_TYPE_NONE||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS_GROUP&&node->io_lock.handle==thread->process->process_group->handle.rb_node.key)||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS&&node->io_lock.handle==thread->process->handle.rb_node.key)||(node->io_lock.type==VFS_NODE_LOCK_TYPE_THREAD&&node->io_lock.handle==thread->handle.rb_node.key));
	rwlock_release_read(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC void vfs_lock_lock(vfs_node_t* node,u32 type,handle_id_t handle){
	rwlock_acquire_write(&(node->io_lock.lock));
	node->io_lock.type=type;
	node->io_lock.handle=handle;
	rwlock_release_write(&(node->io_lock.lock));
}



KERNEL_PUBLIC void vfs_lock_unlock(vfs_node_t* node,u32 type){
	rwlock_acquire_write(&(node->io_lock.lock));
	node->io_lock.type=VFS_NODE_LOCK_TYPE_NONE;
	node->io_lock.handle=0;
	rwlock_release_write(&(node->io_lock.lock));
}
