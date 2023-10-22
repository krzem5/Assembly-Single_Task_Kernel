#include <kernel/fd/fd.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fd"



static pmm_counter_descriptor_t _fd_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_fd");
static pmm_counter_descriptor_t _fd_iterator_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_fd_iterator");
static omm_allocator_t _fd_allocator=OMM_ALLOCATOR_INIT_STRUCT("fd",sizeof(fd_t),8,4,&_fd_omm_pmm_counter);
static omm_allocator_t _fd_iterator_allocator=OMM_ALLOCATOR_INIT_STRUCT("fd_iterator",sizeof(fd_iterator_t),8,4,&_fd_iterator_omm_pmm_counter);



static HANDLE_DECLARE_TYPE(FD,{
	fd_t* data=handle->object;
	omm_dealloc(&_fd_allocator,data);
});
static HANDLE_DECLARE_TYPE(FD_ITERATOR,{
	fd_iterator_t* data=handle->object;
	omm_dealloc(&_fd_iterator_allocator,data);
});



s64 fd_open(handle_id_t root,const char* path,u32 length,u32 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY))){
		return FD_ERROR_INVALID_FLAGS;
	}
	char buffer[4096];
	if (length>4095){
		return FD_ERROR_INVALID_POINTER;
	}
	memcpy(buffer,path,length);
	buffer[length]=0;
	handle_t* root_handle=NULL;
	vfs_node_t* root_node=NULL;
	if (root){
		root_handle=handle_lookup_and_acquire(root,HANDLE_TYPE_FD);
		if (!root_handle){
			return FD_ERROR_INVALID_FD;
		}
		root_node=((fd_t*)(root_handle->object))->node;
	}
	if (flags&FD_FLAG_CREATE){
		panic("FD_FLAG_CREATE");
	}
	vfs_node_t* node=vfs_lookup(root_node,buffer);
	if (root_handle){
		handle_release(root_handle);
	}
	if (!node){
		return FD_ERROR_NOT_FOUND;
	}
	fd_t* out=omm_alloc(&_fd_allocator);
	handle_new(out,HANDLE_TYPE_FD,&(out->handle));
	lock_init(&(out->lock));
	out->node=node;
	out->offset=((flags&FD_FLAG_APPEND)?vfs_node_resize(node,0,VFS_NODE_FLAG_RESIZE_RELATIVE):0);
	out->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE);
	return out->handle.rb_node.key;
}



s64 fd_close(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	handle_release(fd_handle);
	handle_release(fd_handle);
	return 0;
}



s64 fd_read(handle_id_t fd,void* buffer,u64 count){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_t* data=fd_handle->object;
	if (!(data->flags&FD_FLAG_READ)){
		handle_release(fd_handle);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs_node_read(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return count;
}



s64 fd_write(handle_id_t fd,const void* buffer,u64 count){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_t* data=fd_handle->object;
	if (!(data->flags&FD_FLAG_WRITE)){
		handle_release(fd_handle);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs_node_write(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return count;
}



s64 fd_seek(handle_id_t fd,u64 offset,u32 type){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_t* data=fd_handle->object;
	lock_acquire_exclusive(&(data->lock));
	switch (type){
		case FD_SEEK_SET:
			data->offset=offset;
			break;
		case FD_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD_SEEK_END:
			data->offset=vfs_node_resize(data->node,0,VFS_NODE_FLAG_RESIZE_RELATIVE);
			break;
		default:
			lock_release_exclusive(&(data->lock));
			handle_release(fd_handle);
			return FD_ERROR_INVALID_FLAGS;
	}
	u64 out=data->offset;
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return out;
}



s64 fd_resize(handle_id_t fd,u64 size,u32 flags){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_t* data=fd_handle->object;
	lock_acquire_exclusive(&(data->lock));
	s64 out=(vfs_node_resize(data->node,size,0)?0:FD_ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return out;
}



s64 fd_stat(handle_id_t fd,fd_stat_t* out){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_t* data=fd_handle->object;
	lock_acquire_exclusive(&(data->lock));
	out->type=data->node->flags&VFS_NODE_TYPE_MASK;
	out->name_length=data->node->name->length;
	out->fs_handle=data->node->fs->handle.rb_node.key;
	out->size=vfs_node_resize(data->node,0,VFS_NODE_FLAG_RESIZE_RELATIVE);
	memcpy(out->name,data->node->name->data,data->node->name->length+1);
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return 0;
}



s64 fd_dup(handle_id_t fd,u32 flags){
	panic("fd_dup");
}



s64 fd_path(handle_id_t fd,char* buffer,u32 buffer_length){
	panic("fd_path");
}



s64 fd_iter_start(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_t* data=fd_handle->object;
	lock_acquire_exclusive(&(data->lock));
	vfs_name_t* current_name;
	u64 pointer=vfs_node_iterate(data->node,0,&current_name);
	if (!pointer){
		lock_release_exclusive(&(data->lock));
		handle_release(fd_handle);
		return -1;
	}
	fd_iterator_t* out=omm_alloc(&_fd_iterator_allocator);
	handle_new(out,HANDLE_TYPE_FD_ITERATOR,&(out->handle));
	lock_init(&(out->lock));
	out->node=data->node;
	out->pointer=pointer;
	out->current_name=current_name;
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return out->handle.rb_node.key;
}



s64 fd_iter_get(handle_id_t iterator,char* buffer,u32 buffer_length){
	handle_t* fd_iterator_handle=handle_lookup_and_acquire(iterator,HANDLE_TYPE_FD_ITERATOR);
	if (!fd_iterator_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_iterator_t* data=fd_iterator_handle->object;
	lock_acquire_shared(&(data->lock));
	if (data->current_name){
		if (buffer_length>data->current_name->length+1){
			buffer_length=data->current_name->length+1;
		}
		if (buffer_length){
			buffer_length--;
			memcpy(buffer,data->current_name->data,buffer_length);
			buffer[buffer_length]=0;
		}
	}
	else{
		buffer_length=0;
	}
	lock_release_shared(&(data->lock));
	handle_release(fd_iterator_handle);
	return buffer_length;
}



s64 fd_iter_next(handle_id_t iterator){
	handle_t* fd_iterator_handle=handle_lookup_and_acquire(iterator,HANDLE_TYPE_FD_ITERATOR);
	if (!fd_iterator_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_iterator_t* data=fd_iterator_handle->object;
	lock_acquire_exclusive(&(data->lock));
	s64 out=-1;
	if (data->current_name){
		vfs_name_dealloc(data->current_name);
		data->pointer=vfs_node_iterate(data->node,data->pointer,&(data->current_name));
		if (!data->pointer){
			handle_release(fd_iterator_handle);
		}
		else{
			out=fd_iterator_handle->rb_node.key;
		}
	}
	lock_release_exclusive(&(data->lock));
	handle_release(fd_iterator_handle);
	return out;
}



s64 fd_iter_stop(handle_id_t iterator){
	panic("fd_iter_stop");
}
