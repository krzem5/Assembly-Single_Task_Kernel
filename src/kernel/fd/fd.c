#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/mutex.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/permissions.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fd"



static omm_allocator_t* _fd_allocator=NULL;
static omm_allocator_t* _fd_iterator_allocator=NULL;
static handle_type_t _fd_handle_type=0;
static handle_type_t _fd_iterator_handle_type=0;



static void _fd_handle_destructor(handle_t* handle){
	fd_t* data=KERNEL_CONTAINEROF(handle,fd_t,handle);
	if (!data->node->rc&&(data->flags&FD_FLAG_DELETE_ON_EXIT)){
		panic("FD_FLAG_DELETE_ON_EXIT");
	}
	mutex_deinit(data->lock);
	omm_dealloc(_fd_allocator,data);
}



static void _fd_iterator_handle_destructor(handle_t* handle){
	fd_iterator_t* data=KERNEL_CONTAINEROF(handle,fd_iterator_t,handle);
	if (data->current_name){
		smm_dealloc(data->current_name);
	}
	mutex_deinit(data->lock);
	omm_dealloc(_fd_iterator_allocator,data);
}



KERNEL_INIT(){
	LOG("Initializing file descriptors...");
	_fd_allocator=omm_init("kernel.fd",sizeof(fd_t),8,4);
	rwlock_init(&(_fd_allocator->lock));
	_fd_iterator_allocator=omm_init("kernel.fd.iterator",sizeof(fd_iterator_t),8,4);
	rwlock_init(&(_fd_iterator_allocator->lock));
	_fd_handle_type=handle_alloc("kernel.fd",_fd_handle_destructor);
	_fd_iterator_handle_type=handle_alloc("kernel.fd.iterator",_fd_iterator_handle_destructor);
}



KERNEL_PUBLIC error_t fd_from_node(vfs_node_t* node,u32 flags){
	node->rc++;
	fd_t* out=omm_alloc(_fd_allocator);
	handle_new(_fd_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,FD_ACL_FLAG_STAT|FD_ACL_FLAG_DUP|FD_ACL_FLAG_IO|FD_ACL_FLAG_CLOSE);
	out->lock=mutex_init();
	out->node=node;
	out->offset=((flags&FD_FLAG_APPEND)?vfs_node_resize(node,0,VFS_NODE_FLAG_RESIZE_RELATIVE):0);
	out->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_DELETE_ON_EXIT);
	u8 permissions=vfs_permissions_get(node,THREAD_DATA->process->uid,THREAD_DATA->process->gid);
	if (!(permissions&VFS_PERMISSION_READ)||(node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		out->flags&=~FD_FLAG_READ;
	}
	if (!(permissions&VFS_PERMISSION_WRITE)||(node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		out->flags&=~FD_FLAG_WRITE;
	}
	handle_finish_setup(&(out->handle));
	return out->handle.rb_node.key;
}



KERNEL_PUBLIC vfs_node_t* fd_get_node(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return NULL;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	return data->node;
}



error_t syscall_fd_open(handle_id_t root,KERNEL_USER_POINTER const char* path,u32 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY|FD_FLAG_IGNORE_LINKS|FD_FLAG_DELETE_ON_EXIT|FD_FLAG_EXCLUSIVE_CREATE|FD_FLAG_LINK))){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if ((flags&(FD_FLAG_DIRECTORY|FD_FLAG_LINK))==(FD_FLAG_DIRECTORY|FD_FLAG_LINK)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	u64 path_length=syscall_get_string_length((const char*)path);
	if (!path_length||path_length>4095){
		return ERROR_INVALID_ARGUMENT(1);
	}
	char buffer[4096];
	mem_copy(buffer,(const char*)path,path_length);
	buffer[path_length]=0;
	handle_t* root_handle=NULL;
	vfs_node_t* root_node=NULL;
	if (root){
		root_handle=handle_lookup_and_acquire(root,_fd_handle_type);
		if (!root_handle){
			return ERROR_INVALID_HANDLE;
		}
		root_node=KERNEL_CONTAINEROF(root_handle,fd_t,handle)->node;
	}
	error_t error=ERROR_NOT_FOUND;
	vfs_node_t* node;
	if (flags&FD_FLAG_CREATE){
		vfs_node_t* parent;
		const char* child_name;
		node=vfs_lookup_for_creation(root_node,buffer,VFS_LOOKUP_FLAG_CHECK_PERMISSIONS|((flags&FD_FLAG_IGNORE_LINKS)?0:VFS_LOOKUP_FLAG_FOLLOW_LINKS),THREAD_DATA->process->uid,THREAD_DATA->process->gid,&parent,&child_name);
		if (!node&&parent&&child_name){
			SMM_TEMPORARY_STRING child_name_string=smm_alloc(child_name,0);
			node=vfs_node_create(NULL,parent,child_name_string,((flags&FD_FLAG_DIRECTORY)?VFS_NODE_TYPE_DIRECTORY:((flags&FD_FLAG_LINK)?VFS_NODE_TYPE_LINK:VFS_NODE_TYPE_FILE))|VFS_NODE_FLAG_CREATE);
			if (node){
				node->uid=THREAD_DATA->process->uid;
				node->gid=THREAD_DATA->process->gid;
				node->flags|=(((flags&FD_FLAG_DIRECTORY)?0775:0664)<<VFS_NODE_PERMISSION_SHIFT)|VFS_NODE_FLAG_DIRTY;
				vfs_node_flush(node);
			}
		}
		else if (flags&FD_FLAG_EXCLUSIVE_CREATE){
			node=NULL;
			error=ERROR_ALREADY_PRESENT;
		}
	}
	else{
		node=vfs_lookup(root_node,buffer,VFS_LOOKUP_FLAG_CHECK_PERMISSIONS|((flags&FD_FLAG_IGNORE_LINKS)?0:VFS_LOOKUP_FLAG_FOLLOW_LINKS),THREAD_DATA->process->uid,THREAD_DATA->process->gid);
	}
	if (root_handle){
		handle_release(root_handle);
	}
	return (node?fd_from_node(node,flags):error);
}



error_t syscall_fd_close(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_CLOSE)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	data->node->rc--;
	handle_release(fd_handle);
	handle_release(fd_handle);
	return ERROR_OK;
}



error_t syscall_fd_read(handle_id_t fd,KERNEL_USER_POINTER void* buffer,u64 count,u32 flags){
	if (count>syscall_get_user_pointer_max_length((void*)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (flags&(~(FD_FLAG_NONBLOCKING|FD_FLAG_PIPE_PEEK))){
		return ERROR_INVALID_ARGUMENT(3);
	}
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_IO)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	if (!(data->flags&FD_FLAG_READ)){
		handle_release(fd_handle);
		return ERROR_UNSUPPORTED_OPERATION;
	}
	mutex_acquire(data->lock);
	count=vfs_node_read(data->node,data->offset,(void*)buffer,count,((flags&FD_FLAG_NONBLOCKING)?VFS_NODE_FLAG_NONBLOCKING:0)|((flags&FD_FLAG_PIPE_PEEK)?VFS_NODE_FLAG_PIPE_PEEK:0));
	data->offset+=count;
	mutex_release(data->lock);
	handle_release(fd_handle);
	return count;
}



error_t syscall_fd_write(handle_id_t fd,KERNEL_USER_POINTER const void* buffer,u64 count,u32 flags){
	if (count>syscall_get_user_pointer_max_length((const void*)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (flags&(~FD_FLAG_NONBLOCKING)){
		return ERROR_INVALID_ARGUMENT(3);
	}
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_IO)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	if (!(data->flags&FD_FLAG_WRITE)){
		handle_release(fd_handle);
		return ERROR_UNSUPPORTED_OPERATION;
	}
	mutex_acquire(data->lock);
	count=vfs_node_write(data->node,data->offset,(const void*)buffer,count,((flags&FD_FLAG_NONBLOCKING)?VFS_NODE_FLAG_NONBLOCKING:0));
	data->offset+=count;
	mutex_release(data->lock);
	handle_release(fd_handle);
	return count;
}



error_t syscall_fd_seek(handle_id_t fd,s64 offset,u32 type){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_IO)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	switch (type){
		case FD_SEEK_SET:
			data->offset=offset;
			break;
		case FD_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD_SEEK_END:
			data->offset=vfs_node_resize(data->node,0,VFS_NODE_FLAG_RESIZE_RELATIVE)-offset;
			break;
		default:
			mutex_release(data->lock);
			handle_release(fd_handle);
			return ERROR_INVALID_ARGUMENT(2);
	}
	u64 out=data->offset;
	mutex_release(data->lock);
	handle_release(fd_handle);
	return out;
}



error_t syscall_fd_resize(handle_id_t fd,u64 size,u32 flags){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_IO)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	error_t out=(vfs_node_resize(data->node,size,0)?0:ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	mutex_release(data->lock);
	handle_release(fd_handle);
	return out;
}



error_t syscall_fd_stat(handle_id_t fd,KERNEL_USER_POINTER fd_stat_t* out,u32 buffer_length){
	if (buffer_length<sizeof(fd_stat_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (buffer_length>syscall_get_user_pointer_max_length((void*)out)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_STAT)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	out->type=data->node->flags&VFS_NODE_TYPE_MASK;
	out->flags=((data->node->flags&VFS_NODE_FLAG_VIRTUAL)?FD_STAT_FLAG_VIRTUAL:0);
	out->permissions=(data->node->flags&VFS_NODE_PERMISSION_MASK)>>VFS_NODE_PERMISSION_SHIFT;
	out->name_length=data->node->name->length;
	out->fs_handle=(data->node->fs?data->node->fs->handle.rb_node.key:0);
	out->size=vfs_node_resize(data->node,0,VFS_NODE_FLAG_RESIZE_RELATIVE);
	out->time_access=data->node->time_access;
	out->time_modify=data->node->time_modify;
	out->time_change=data->node->time_change;
	out->time_birth=data->node->time_birth;
	out->gid=data->node->gid;
	out->uid=data->node->uid;
	mem_copy((char*)(out->name),data->node->name->data,data->node->name->length+1);
	mutex_release(data->lock);
	handle_release(fd_handle);
	return ERROR_OK;
}



error_t syscall_fd_dup(handle_id_t fd,u32 flags){
	panic("fd_dup");
}



error_t syscall_fd_path(handle_id_t fd,KERNEL_USER_POINTER char* buffer,u32 buffer_length){
	if (buffer_length>syscall_get_user_pointer_max_length((char*)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (buffer_length<2){
		return ERROR_NO_SPACE;
	}
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_STAT)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	u32 out=vfs_path(data->node,(char*)buffer,buffer_length);
	mutex_release(data->lock);
	handle_release(fd_handle);
	return (!out&&buffer_length?ERROR_NO_SPACE:out);
}



error_t syscall_fd_iter_start(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_STAT)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	if (!(vfs_permissions_get(data->node,THREAD_DATA->process->uid,THREAD_DATA->process->gid)&VFS_PERMISSION_READ)){
		mutex_release(data->lock);
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	string_t* current_name;
	u64 pointer=((vfs_permissions_get(data->node,THREAD_DATA->process->uid,THREAD_DATA->process->gid)&VFS_PERMISSION_EXEC)?vfs_node_iterate(data->node,0,&current_name):0);
	if (!pointer){
		mutex_release(data->lock);
		handle_release(fd_handle);
		return ERROR_NO_DATA;
	}
	fd_iterator_t* out=omm_alloc(_fd_iterator_allocator);
	handle_new(_fd_iterator_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,FD_ITERATOR_ACL_FLAG_ACCESS);
	out->lock=mutex_init();
	out->node=data->node;
	out->pointer=pointer;
	out->current_name=current_name;
	handle_finish_setup(&(out->handle));
	mutex_release(data->lock);
	handle_release(fd_handle);
	return out->handle.rb_node.key;
}



error_t syscall_fd_iter_get(handle_id_t iterator,KERNEL_USER_POINTER char* buffer,u32 buffer_length){
	if (buffer_length>syscall_get_user_pointer_max_length((char*)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* fd_iterator_handle=handle_lookup_and_acquire(iterator,_fd_iterator_handle_type);
	if (!fd_iterator_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_iterator_t* data=KERNEL_CONTAINEROF(fd_iterator_handle,fd_iterator_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ITERATOR_ACL_FLAG_ACCESS)){
		handle_release(fd_iterator_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	if (data->current_name){
		if (buffer_length>data->current_name->length+1){
			buffer_length=data->current_name->length+1;
		}
		if (buffer_length){
			buffer_length--;
			mem_copy((char*)buffer,data->current_name->data,buffer_length);
			buffer[buffer_length]=0;
		}
	}
	else{
		buffer_length=0;
	}
	mutex_release(data->lock);
	handle_release(fd_iterator_handle);
	return buffer_length;
}



error_t syscall_fd_iter_next(handle_id_t iterator){
	handle_t* fd_iterator_handle=handle_lookup_and_acquire(iterator,_fd_iterator_handle_type);
	if (!fd_iterator_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_iterator_t* data=KERNEL_CONTAINEROF(fd_iterator_handle,fd_iterator_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ITERATOR_ACL_FLAG_ACCESS)){
		handle_release(fd_iterator_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	s64 out=ERROR_NO_DATA;
	if (data->current_name){
		smm_dealloc(data->current_name);
		data->current_name=NULL;
		data->pointer=((vfs_permissions_get(data->node,THREAD_DATA->process->uid,THREAD_DATA->process->gid)&VFS_PERMISSION_EXEC)?vfs_node_iterate(data->node,data->pointer,&(data->current_name)):0);
		if (!data->pointer){
			handle_release(fd_iterator_handle);
		}
		else{
			out=fd_iterator_handle->rb_node.key;
		}
	}
	mutex_release(data->lock);
	handle_release(fd_iterator_handle);
	return out;
}



error_t syscall_fd_iter_stop(handle_id_t iterator){
	handle_t* fd_iterator_handle=handle_lookup_and_acquire(iterator,_fd_iterator_handle_type);
	if (!fd_iterator_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_iterator_t* data=KERNEL_CONTAINEROF(fd_iterator_handle,fd_iterator_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ITERATOR_ACL_FLAG_ACCESS)){
		handle_release(fd_iterator_handle);
		return ERROR_DENIED;
	}
	handle_release(fd_iterator_handle);
	handle_release(fd_iterator_handle);
	return ERROR_OK;
}
