#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/lock/mutex.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/lock.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/permissions.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fd"



#define STREAM_MAX_BUFFER_SIZE 0x100000 // 1 Mb



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _fd_stream_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fd_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fd_iterator_allocator=NULL;
static handle_type_t KERNEL_INIT_WRITE _fd_handle_type=0;
static handle_type_t KERNEL_INIT_WRITE _fd_iterator_handle_type=0;



static void _fd_handle_destructor(handle_t* handle){
	fd_t* data=KERNEL_CONTAINEROF(handle,fd_t,handle);
	if (data->flags&FD_FLAG_CLOSE_PIPE){
		pipe_close(data->node);
	}
	vfs_node_unref(data->node);
	mutex_deinit(data->lock);
	omm_dealloc(_fd_allocator,data);
}



static void _fd_iterator_handle_destructor(handle_t* handle){
	fd_iterator_t* data=KERNEL_CONTAINEROF(handle,fd_iterator_t,handle);
	if (data->current_name){
		smm_dealloc(data->current_name);
	}
	vfs_node_unref(data->node);
	mutex_deinit(data->lock);
	omm_dealloc(_fd_iterator_allocator,data);
}



KERNEL_INIT(){
	LOG("Initializing file descriptors...");
	_fd_stream_pmm_counter=pmm_alloc_counter("kernel.fd.stream");
	_fd_allocator=omm_init("kernel.fd",sizeof(fd_t),8,4);
	rwlock_init(&(_fd_allocator->lock));
	_fd_iterator_allocator=omm_init("kernel.fd.iterator",sizeof(fd_iterator_t),8,4);
	rwlock_init(&(_fd_iterator_allocator->lock));
	_fd_handle_type=handle_alloc("kernel.fd",0,_fd_handle_destructor);
	_fd_iterator_handle_type=handle_alloc("kernel.fd.iterator",0,_fd_iterator_handle_destructor);
}



KERNEL_PUBLIC error_t fd_from_node(vfs_node_t* node,u32 flags){
	vfs_node_ref(node);
	fd_t* out=omm_alloc(_fd_allocator);
	handle_new(_fd_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,FD_ACL_FLAG_STAT|FD_ACL_FLAG_DUP|FD_ACL_FLAG_IO|FD_ACL_FLAG_CLOSE);
	out->lock=mutex_init("kernel.fd");
	out->node=node;
	out->offset=((flags&FD_FLAG_APPEND)?vfs_node_resize(node,0,VFS_NODE_FLAG_RESIZE_RELATIVE):0);
	out->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_CLOSE_PIPE);
	u8 permissions=vfs_permissions_get(node,THREAD_DATA->process->uid,THREAD_DATA->process->gid);
	if (!(permissions&VFS_PERMISSION_READ)||(node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		out->flags&=~FD_FLAG_READ;
	}
	if (!(permissions&VFS_PERMISSION_WRITE)||(node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		out->flags&=~(FD_FLAG_WRITE|FD_FLAG_CLOSE_PIPE);
	}
	if (flags&FD_FLAG_DELETE_ON_EXIT){
		node->flags|=VFS_NODE_FLAG_TEMPORARY;
	}
	return out->handle.rb_node.key;
}



KERNEL_PUBLIC vfs_node_t* fd_get_node(handle_id_t fd,u64* acl){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return NULL;
	}
	if (acl){
		*acl=acl_get(fd_handle->acl,THREAD_DATA->process);
	}
	vfs_node_t* out=KERNEL_CONTAINEROF(fd_handle,fd_t,handle)->node;
	vfs_node_ref(out);
	handle_release(fd_handle);
	return out;
}



void fd_allow_dup(handle_id_t fd,process_t* process){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return;
	}
	if (fd_handle->acl){
		acl_set(fd_handle->acl,process,0,FD_ACL_FLAG_DUP);
	}
	handle_release(fd_handle);
}



void fd_ref(handle_id_t fd){
	handle_lookup_and_acquire(fd,_fd_handle_type);
}



void fd_unref(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return;
	}
	handle_release(fd_handle);
	handle_release(fd_handle);
}



error_t syscall_fd_open(handle_id_t root,KERNEL_USER_POINTER const char* path,u32 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY|FD_FLAG_IGNORE_LINKS|FD_FLAG_DELETE_ON_EXIT|FD_FLAG_EXCLUSIVE_CREATE|FD_FLAG_LINK|FD_FLAG_CLOSE_PIPE|FD_FLAG_FIND_LINKS))){
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
	vfs_node_t* node=NULL;
	if (flags&FD_FLAG_CREATE){
		vfs_node_t* parent;
		const char* child_name;
		node=vfs_lookup_for_creation(root_node,buffer,VFS_LOOKUP_FLAG_CHECK_PERMISSIONS|((flags&FD_FLAG_IGNORE_LINKS)?0:VFS_LOOKUP_FLAG_FOLLOW_LINKS)|((flags&FD_FLAG_FIND_LINKS)?VFS_LOOKUP_FLAG_FIND_LINKS:0),THREAD_DATA->process->uid,THREAD_DATA->process->gid,&parent,&child_name);
		if (node==VFS_LOOKUP_LINK_FOUND){
			node=NULL;
			error=ERROR_LINK_FOUND;
		}
		else if (!node&&parent&&child_name){
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
		if (parent){
			vfs_node_unref(parent);
		}
	}
	else{
		node=vfs_lookup(root_node,buffer,VFS_LOOKUP_FLAG_CHECK_PERMISSIONS|((flags&FD_FLAG_IGNORE_LINKS)?0:VFS_LOOKUP_FLAG_FOLLOW_LINKS)|((flags&FD_FLAG_FIND_LINKS)?VFS_LOOKUP_FLAG_FIND_LINKS:0),THREAD_DATA->process->uid,THREAD_DATA->process->gid);
		if (node==VFS_LOOKUP_LINK_FOUND){
			node=NULL;
			error=ERROR_LINK_FOUND;
		}
	}
	if (root_handle){
		handle_release(root_handle);
	}
	if (!node){
		return error;
	}
	error_t out=fd_from_node(node,flags);
	vfs_node_unref(node);
	return out;
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
	handle_list_pop(&(THREAD_DATA->process->handle_list),fd_handle);
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
	if (!vfs_lock_verify_thread(data->node,THREAD_DATA->header.current_thread)){
		mutex_release(data->lock);
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
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
	if (!vfs_lock_verify_thread(data->node,THREAD_DATA->header.current_thread)){
		mutex_release(data->lock);
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	count=vfs_node_write(data->node,data->offset,(const void*)buffer,count,((flags&FD_FLAG_NONBLOCKING)?VFS_NODE_FLAG_NONBLOCKING:0)|VFS_NODE_FLAG_GROW);
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
	if (!vfs_lock_verify_thread(data->node,THREAD_DATA->header.current_thread)){
		mutex_release(data->lock);
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	error_t out=(vfs_node_resize(data->node,size,0)||!size?0:ERROR_NO_SPACE);
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
	out->lock_handle=data->node->io_lock.handle;
	mem_copy((char*)(out->name),data->node->name->data,data->node->name->length+1);
	mutex_release(data->lock);
	handle_release(fd_handle);
	return ERROR_OK;
}



error_t syscall_fd_dup(handle_id_t fd,u32 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_CLOSE_PIPE))){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (fd==FD_DUP_CWD){
		return fd_from_node(THREAD_DATA->process->vfs_cwd,flags);
	}
	else if (fd==FD_DUP_STDIN&&THREAD_DATA->process->fd_stdin){
		fd=THREAD_DATA->process->fd_stdin;
	}
	else if (fd==FD_DUP_STDOUT&&THREAD_DATA->process->fd_stdout){
		fd=THREAD_DATA->process->fd_stdout;
	}
	else if (fd==FD_DUP_STDERR&&THREAD_DATA->process->fd_stderr){
		fd=THREAD_DATA->process->fd_stderr;
	}
	handle_t* fd_handle=handle_lookup_and_acquire(fd,_fd_handle_type);
	if (!fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* data=KERNEL_CONTAINEROF(fd_handle,fd_t,handle);
	if (!(acl_get(data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_DUP)){
		handle_release(fd_handle);
		return ERROR_DENIED;
	}
	mutex_acquire(data->lock);
	vfs_node_ref(data->node);
	fd_t* out=omm_alloc(_fd_allocator);
	handle_new(_fd_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,FD_ACL_FLAG_STAT|FD_ACL_FLAG_DUP|FD_ACL_FLAG_IO|FD_ACL_FLAG_CLOSE);
	out->lock=mutex_init("kernel.fd");
	out->node=data->node;
	out->offset=data->offset;
	out->flags=(data->flags|FD_FLAG_CLOSE_PIPE)&flags;
	mutex_release(data->lock);
	handle_release(fd_handle);
	return out->handle.rb_node.key;
}



error_t syscall_fd_link(handle_id_t parent,handle_id_t fd){
	panic("syscall_fd_link");
}



error_t syscall_fd_unlink(handle_id_t fd){
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
	error_t out=ERROR_DENIED;
	if (vfs_permissions_get(data->node,THREAD_DATA->process->uid,THREAD_DATA->process->gid)&VFS_PERMISSION_WRITE){
		out=(vfs_node_unlink(data->node)?ERROR_OK:ERROR_FAILED);
	}
	mutex_release(data->lock);
	handle_release(fd_handle);
	return out;
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



error_t syscall_fd_stream(handle_id_t src_fd,KERNEL_USER_POINTER const handle_id_t* dst_fds,u32 dst_fd_count,u64 length){
	if (!dst_fd_count){
		return 0;
	}
	if (dst_fd_count*sizeof(handle_id_t)>syscall_get_user_pointer_max_length((void*)dst_fds)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* src_fd_handle=handle_lookup_and_acquire(src_fd,_fd_handle_type);
	if (!src_fd_handle){
		return ERROR_INVALID_HANDLE;
	}
	fd_t* src_fd_data=KERNEL_CONTAINEROF(src_fd_handle,fd_t,handle);
	if (!(acl_get(src_fd_data->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_IO)){
		handle_release(src_fd_handle);
		return ERROR_DENIED;
	}
	if (!(src_fd_data->flags&FD_FLAG_READ)){
		handle_release(src_fd_handle);
		return ERROR_UNSUPPORTED_OPERATION;
	}
	u64 out=0;
	fd_t** dst_fd_data=amm_alloc(dst_fd_count*sizeof(fd_t*));
	for (u32 i=0;i<dst_fd_count;i++){
		handle_t* handle=handle_lookup_and_acquire(dst_fds[i],_fd_handle_type);
		if (!handle){
			out=ERROR_INVALID_HANDLE;
			goto _cleanup;
		}
		dst_fd_data[i]=KERNEL_CONTAINEROF(handle,fd_t,handle);
		if (!(acl_get(dst_fd_data[i]->handle.acl,THREAD_DATA->process)&FD_ACL_FLAG_IO)){
			out=ERROR_DENIED;
			goto _cleanup;
		}
		if (!(dst_fd_data[i]->flags&FD_FLAG_WRITE)){
			out=ERROR_UNSUPPORTED_OPERATION;
			goto _cleanup;
		}
	}
	u64 buffer_length=(!length||length>STREAM_MAX_BUFFER_SIZE?STREAM_MAX_BUFFER_SIZE:length);
	u64 buffer=pmm_alloc(pmm_align_up_address(buffer_length)>>PAGE_SIZE_SHIFT,_fd_stream_pmm_counter,0);
	while (1){
		mutex_acquire(src_fd_data->lock);
		u64 count=vfs_node_read(src_fd_data->node,src_fd_data->offset,(void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),(length&&buffer_length>length?length:buffer_length),0);
		src_fd_data->offset+=count;
		mutex_release(src_fd_data->lock);
		if (!count){
			break;
		}
		for (u32 i=0;i<dst_fd_count;i++){
			mutex_acquire(dst_fd_data[i]->lock);
			u64 write_count=count;
			while (write_count){
				u64 chunk_size=vfs_node_write(dst_fd_data[i]->node,dst_fd_data[i]->offset,(const void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),write_count,VFS_NODE_FLAG_GROW);
				if (!chunk_size){
					break;
				}
				dst_fd_data[i]->offset+=chunk_size;
				write_count-=chunk_size;
			}
			mutex_release(dst_fd_data[i]->lock);
		}
		out+=count;
		if (length){
			length-=count;
			if (!length){
				break;
			}
		}
	}
	pmm_dealloc(buffer,pmm_align_up_address(buffer_length)>>PAGE_SIZE_SHIFT,_fd_stream_pmm_counter);
_cleanup:
	for (u32 i=0;i<dst_fd_count;i++){
		if (!dst_fd_data[i]){
			continue;
		}
		handle_release(&(dst_fd_data[i]->handle));
	}
	amm_dealloc(dst_fd_data);
	handle_release(src_fd_handle);
	return out;
}



error_t syscall_fd_lock(handle_id_t fd,handle_id_t handle){
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
	error_t out=(vfs_lock_lock_thread(data->node,THREAD_DATA->header.current_thread,handle)?ERROR_OK:ERROR_DENIED);
	mutex_release(data->lock);
	handle_release(fd_handle);
	return out;
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
	vfs_node_ref(data->node);
	fd_iterator_t* out=omm_alloc(_fd_iterator_allocator);
	handle_new(_fd_iterator_handle_type,&(out->handle));
	handle_list_push(&(THREAD_DATA->process->handle_list),&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,THREAD_DATA->process,0,FD_ITERATOR_ACL_FLAG_ACCESS);
	out->lock=mutex_init("kernel.fd.iterator");
	out->node=data->node;
	out->pointer=pointer;
	out->current_name=current_name;
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
			handle_list_pop(&(THREAD_DATA->process->handle_list),fd_iterator_handle);
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
	handle_list_pop(&(THREAD_DATA->process->handle_list),fd_iterator_handle);
	handle_release(fd_iterator_handle);
	handle_release(fd_iterator_handle);
	return ERROR_OK;
}
