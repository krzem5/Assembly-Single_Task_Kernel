#include <kernel/lock/rwlock.h>
#include <kernel/mp/process.h>
#include <kernel/mp/process_group.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>



static bool _check_process_group(vfs_node_t* node,process_group_t* process_group){
	return (node->io_lock.type==VFS_NODE_LOCK_TYPE_NONE||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS_GROUP&&node->io_lock.handle==process_group->handle.rb_node.key));
}



static bool _check_process(vfs_node_t* node,process_t* process){
	return (node->io_lock.type==VFS_NODE_LOCK_TYPE_NONE||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS_GROUP&&node->io_lock.handle==process->process_group->handle.rb_node.key)||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS&&node->io_lock.handle==process->handle.rb_node.key));
}



static bool _check_thread(vfs_node_t* node,thread_t* thread){
	return (node->io_lock.type==VFS_NODE_LOCK_TYPE_NONE||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS_GROUP&&node->io_lock.handle==thread->process->process_group->handle.rb_node.key)||(node->io_lock.type==VFS_NODE_LOCK_TYPE_PROCESS&&node->io_lock.handle==thread->process->handle.rb_node.key)||(node->io_lock.type==VFS_NODE_LOCK_TYPE_THREAD&&node->io_lock.handle==thread->handle.rb_node.key));
}



static void _unlock_if_handle_closed(vfs_node_t* node){
	if (!node->io_lock.handle){
		return;
	}
	handle_t* handle=handle_lookup_and_acquire(node->io_lock.handle,HANDLE_TYPE_ANY);
	if (handle){
		handle_release(handle);
		return;
	}
	node->io_lock.type=VFS_NODE_LOCK_TYPE_NONE;
	node->io_lock.handle=0;
}



static u32 _get_type_from_handle(handle_id_t handle){
	if (!handle){
		return VFS_NODE_LOCK_TYPE_NONE;
	}
	if (HANDLE_ID_GET_TYPE(handle)==thread_handle_type){
		return VFS_NODE_LOCK_TYPE_THREAD;
	}
	if (HANDLE_ID_GET_TYPE(handle)==process_handle_type){
		return VFS_NODE_LOCK_TYPE_PROCESS;
	}
	if (HANDLE_ID_GET_TYPE(handle)==process_group_handle_type){
		return VFS_NODE_LOCK_TYPE_PROCESS_GROUP;
	}
	return VFS_NODE_LOCK_TYPE_INVALID;
}



KERNEL_PUBLIC void vfs_lock_init(vfs_node_t* node){
	rwlock_init(&(node->io_lock.lock));
	node->io_lock.type=VFS_NODE_LOCK_TYPE_NONE;
	node->io_lock.handle=0;
}



KERNEL_PUBLIC bool vfs_lock_verify_process_group(vfs_node_t* node,process_group_t* process_group){
	rwlock_acquire_read(&(node->io_lock.lock));
	_unlock_if_handle_closed(node);
	bool out=_check_process_group(node,process_group);
	rwlock_release_read(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_verify_process(vfs_node_t* node,process_t* process){
	rwlock_acquire_read(&(node->io_lock.lock));
	_unlock_if_handle_closed(node);
	bool out=_check_process(node,process);
	rwlock_release_read(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_verify_thread(vfs_node_t* node,thread_t* thread){
	rwlock_acquire_read(&(node->io_lock.lock));
	_unlock_if_handle_closed(node);
	bool out=_check_thread(node,thread);
	rwlock_release_read(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_lock_process_group(vfs_node_t* node,process_group_t* process_group,handle_id_t handle){
	u32 type=_get_type_from_handle(handle);
	if (type==VFS_NODE_LOCK_TYPE_INVALID){
		return 0;
	}
	rwlock_acquire_write(&(node->io_lock.lock));
	_unlock_if_handle_closed(node);
	bool out=_check_process_group(node,process_group);
	if (out){
		node->io_lock.type=type;
		node->io_lock.handle=handle;
	}
	rwlock_release_write(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_lock_process(vfs_node_t* node,process_t* process,handle_id_t handle){
	u32 type=_get_type_from_handle(handle);
	if (type==VFS_NODE_LOCK_TYPE_INVALID){
		return 0;
	}
	rwlock_acquire_write(&(node->io_lock.lock));
	_unlock_if_handle_closed(node);
	bool out=_check_process(node,process);
	if (out){
		node->io_lock.type=type;
		node->io_lock.handle=handle;
	}
	rwlock_release_write(&(node->io_lock.lock));
	return out;
}



KERNEL_PUBLIC bool vfs_lock_lock_thread(vfs_node_t* node,thread_t* thread,handle_id_t handle){
	u32 type=_get_type_from_handle(handle);
	if (type==VFS_NODE_LOCK_TYPE_INVALID){
		return 0;
	}
	rwlock_acquire_write(&(node->io_lock.lock));
	_unlock_if_handle_closed(node);
	bool out=_check_thread(node,thread);
	if (out){
		node->io_lock.type=type;
		node->io_lock.handle=handle;
	}
	rwlock_release_write(&(node->io_lock.lock));
	return out;
}
