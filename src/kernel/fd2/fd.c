#include <kernel/fd2/fd2.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/node.h>
#include <kernel/vfs2/vfs.h>
#define KERNEL_LOG_NAME "fd"



PMM_DECLARE_COUNTER(OMM_FD2);



static omm_allocator_t _fd2_allocator=OMM_ALLOCATOR_INIT_STRUCT("fd2",sizeof(fd2_t),8,4,PMM_COUNTER_OMM_FD2);



static HANDLE_DECLARE_TYPE(FD2,{
	fd2_t* data=handle->object;
	if (data->flags&FD2_FLAG_DELETE_AT_EXIT){
		panic("FD2_FLAG_DELETE_AT_EXIT");
	}
	omm_dealloc(&_fd2_allocator,data);
});



static handle_id_t _node_to_fd(vfs2_node_t* node,u8 flags){
	fd2_t* out=omm_alloc(&_fd2_allocator);
	handle_new(out,HANDLE_TYPE_FD2,&(out->handle));
	lock_init(&(out->lock));
	out->node=node;
	out->offset=((flags&FD2_FLAG_APPEND)?vfs2_node_resize(node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE):0);
	out->flags=flags&(FD2_FLAG_READ|FD2_FLAG_WRITE);
	return out->handle.rb_node.key;
}



s64 fd2_open(handle_id_t root,const char* path,u32 length,u8 flags){
	if (flags&(~(FD2_FLAG_READ|FD2_FLAG_WRITE|FD2_FLAG_APPEND|FD2_FLAG_CREATE|FD2_FLAG_DIRECTORY|FD2_FLAG_DELETE_AT_EXIT))){
		return FD2_ERROR_INVALID_FLAGS;
	}
	char buffer[4096];
	if (length>4095){
		return FD2_ERROR_INVALID_POINTER;
	}
	memcpy(buffer,path,length);
	buffer[length]=0;
	handle_t* root_handle=NULL;
	vfs2_node_t* root_node=NULL;
	if (root){
		root_handle=handle_lookup_and_acquire(root,HANDLE_TYPE_FD2);
		if (!root_handle){
			return FD2_ERROR_INVALID_FD;
		}
		root_node=((fd2_t*)(root_handle->object))->node;
		if (!root_node){
			return FD2_ERROR_NOT_FOUND;
		}
	}
	if (flags&FD2_FLAG_CREATE){
		panic("FD2_FLAG_CREATE");
	}
	vfs2_node_t* node=vfs2_lookup(root_node,buffer);
	if (root_handle){
		handle_release(root_handle);
	}
	if (!node){
		return FD2_ERROR_NOT_FOUND;
	}
	return _node_to_fd(node,flags);
}



s64 fd2_close(handle_id_t fd){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	handle_release(fd2_handle);
	handle_release(fd2_handle);
	return 0;
}



s64 fd2_delete(handle_id_t fd){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	data->flags|=FD2_FLAG_DELETE_AT_EXIT;
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	panic("fd2_delete");
	handle_release(fd2_handle);
	handle_release(fd2_handle);
	return 0;
}



s64 fd2_read(handle_id_t fd,void* buffer,u64 count){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!(data->flags&FD2_FLAG_READ)){
		handle_release(fd2_handle);
		return FD2_ERROR_UNSUPPORTED_OPERATION;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs2_node_read(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return count;
}



s64 fd2_write(handle_id_t fd,const void* buffer,u64 count){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!(data->flags&FD2_FLAG_WRITE)){
		handle_release(fd2_handle);
		return FD2_ERROR_UNSUPPORTED_OPERATION;
	}
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs2_node_write(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return count;
}



s64 fd2_seek(handle_id_t fd,u64 offset,u8 flags){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	switch (flags){
		case FD2_SEEK_SET:
			data->offset=offset;
			break;
		case FD2_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD2_SEEK_END:
			data->offset=vfs2_node_resize(data->node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE);
			break;
		default:
			lock_release_exclusive(&(data->lock));
			handle_release(fd2_handle);
			return FD2_ERROR_INVALID_FLAGS;
	}
	u64 out=data->offset;
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return out;
}



s64 fd2_resize(handle_id_t fd,u64 size){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	s64 out=(vfs2_node_resize(data->node,size,0)?0:FD2_ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return out;
}



s64 fd2_absolute_path(handle_id_t fd,char* buffer,u32 buffer_length){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	panic("fd2_absolute_path");
	// s64 out=vfs_get_full_path(data->node,buffer,buffer_length);
	s64 out=0;
	handle_release(fd2_handle);
	return out;
}



s64 fd2_stat(handle_id_t fd,fd2_stat_t* out){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	out->type=data->node->flags&VFS2_NODE_TYPE_MASK;
	out->vfs_index=/*data->node->vfs_index*/0xaa;
	out->name_length=data->node->name->length;
	memcpy(out->name,data->node->name->data,64);
	out->size=vfs2_node_resize(data->node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE);
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return 0;
}



s64 fd2_get_relative(handle_id_t fd,u8 relative,u8 flags){
	if (flags&(~(FD2_FLAG_READ|FD2_FLAG_WRITE|FD2_FLAG_APPEND|FD2_FLAG_DELETE_AT_EXIT))){
		return FD2_ERROR_INVALID_FLAGS;
	}
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	if (!data->node){
		handle_release(fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	vfs2_node_t* other=NULL/*vfs_get_relative(data->node,relative)*/;
	panic("fd2_get_relative");
	if (!other){
		handle_release(fd2_handle);
		return FD2_ERROR_NO_RELATIVE;
	}
	handle_release(fd2_handle);
	return _node_to_fd(other,flags);
}



s64 fd2_move(handle_id_t fd,handle_id_t dst_fd){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	handle_t* dst_fd2_handle=handle_lookup_and_acquire(dst_fd,HANDLE_TYPE_FD2);
	if (!dst_fd2_handle){
		handle_release(fd2_handle);
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* dst_data=dst_fd2_handle->object;
	if (!data->node){
		handle_release(fd2_handle);
		handle_release(dst_fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	if (!dst_data->node){
		handle_release(fd2_handle);
		handle_release(dst_fd2_handle);
		return FD2_ERROR_NOT_FOUND;
	}
	if (data->node->fs!=dst_data->node->fs){
		handle_release(fd2_handle);
		handle_release(dst_fd2_handle);
		return FD2_ERROR_DIFFERENT_FS;
	}
	if ((data->node->flags&VFS2_NODE_TYPE_MASK)!=(dst_data->node->flags&VFS2_NODE_TYPE_MASK)){
		handle_release(fd2_handle);
		handle_release(dst_fd2_handle);
		return FD2_ERROR_DIFFERENT_TYPE;
	}
	lock_acquire_exclusive(&(data->lock));
	lock_acquire_exclusive(&(dst_data->lock));
	panic("fd2_move");
	_Bool error=0;
	// _Bool error=((node->flags&VFS2_NODE_TYPE_DIRECTORY)?!!vfs_get_relative(dst_node,VFS_RELATIVE_FIRST_CHILD):!!vfs_get_size(dst_node))||!vfs_move(node,dst_node);
	lock_release_exclusive(&(data->lock));
	lock_release_exclusive(&(dst_data->lock));
	handle_release(fd2_handle);
	handle_release(dst_fd2_handle);
	return (error?FD2_ERROR_NOT_EMPTY:0);
}
