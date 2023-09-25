#include <kernel/fd/fd.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/allocator.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fd"



static omm_allocator_t _fd_allocator=OMM_ALLOCATOR_INIT_STRUCT(sizeof(fd_data_t),8,4);



static HANDLE_DECLARE_TYPE(FD,{
	fd_data_t* data=handle->object;
	if (data->flags&FD_FLAG_DELETE_AT_EXIT){
		vfs_node_t* node=vfs_get_by_id(data->node_id);
		if (node&&(node->type!=VFS_NODE_TYPE_DIRECTORY||!vfs_get_relative(node,VFS_RELATIVE_FIRST_CHILD))){
			vfs_delete(node);
		}
	}
	omm_dealloc(&_fd_allocator,data);
});



static handle_id_t _node_to_fd(vfs_node_t* node,u8 flags){
	fd_data_t* out=omm_alloc(&_fd_allocator);
	handle_new(out,HANDLE_TYPE_FD,&(out->handle));
	lock_init(&(out->lock));
	out->node_id=node->id;
	out->offset=((flags&FD_FLAG_APPEND)?vfs_get_size(node):0);
	out->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE);
	return out->handle.id;
}



s64 fd_open(handle_id_t root,const char* path,u32 length,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY|FD_FLAG_DELETE_AT_EXIT))){
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
		root_node=vfs_get_by_id(((fd_data_t*)(root_handle->object))->node_id);
		if (!root_node){
			return FD_ERROR_NOT_FOUND;
		}
	}
	vfs_node_t* node=vfs_get_by_path(root_node,buffer,((flags&FD_FLAG_CREATE)?((flags&FD_FLAG_DIRECTORY)?VFS_NODE_TYPE_DIRECTORY:VFS_NODE_TYPE_FILE):0));
	if (root_handle){
		handle_release(root_handle);
	}
	if (!node){
		return FD_ERROR_NOT_FOUND;
	}
	return _node_to_fd(node,flags);
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



s64 fd_delete(handle_id_t fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	data->flags|=FD_FLAG_DELETE_AT_EXIT;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	if (node->type==VFS_NODE_TYPE_DIRECTORY&&vfs_get_relative(node,VFS_RELATIVE_FIRST_CHILD)){
		handle_release(fd_handle);
		return FD_ERROR_NOT_EMPTY;
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
	fd_data_t* data=fd_handle->object;
	if (!(data->flags&FD_FLAG_READ)){
		handle_release(fd_handle);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs_read(node,data->offset,buffer,count);
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
	fd_data_t* data=fd_handle->object;
	if (!(data->flags&FD_FLAG_WRITE)){
		handle_release(fd_handle);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs_write(node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return count;
}



s64 fd_seek(handle_id_t fd,u64 offset,u8 flags){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	switch (flags){
		case FD_SEEK_SET:
			data->offset=offset;
			break;
		case FD_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD_SEEK_END:
			data->offset=vfs_get_size(node);
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



s64 fd_resize(handle_id_t fd,u64 size){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	s64 out=(vfs_set_size(node,size)?0:FD_ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return out;
}



s64 fd_absolute_path(handle_id_t fd,char* buffer,u32 buffer_length){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	s64 out=vfs_get_full_path(node,buffer,buffer_length);
	handle_release(fd_handle);
	return out;
}



s64 fd_stat(handle_id_t fd,fd_stat_t* out){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	lock_acquire_exclusive(&(data->lock));
	out->node_id=node->id;
	out->type=node->type;
	out->vfs_index=node->vfs_index;
	out->name_length=node->name_length;
	memcpy(out->name,node->name,64);
	out->size=vfs_get_size(node);
	lock_release_exclusive(&(data->lock));
	handle_release(fd_handle);
	return 0;
}



s64 fd_get_relative(handle_id_t fd,u8 relative,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_DELETE_AT_EXIT))){
		return FD_ERROR_INVALID_FLAGS;
	}
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	vfs_node_t* other=vfs_get_relative(node,relative);
	if (!other){
		handle_release(fd_handle);
		return FD_ERROR_NO_RELATIVE;
	}
	handle_release(fd_handle);
	return _node_to_fd(other,flags);
}



s64 fd_move(handle_id_t fd,handle_id_t dst_fd){
	handle_t* fd_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD);
	if (!fd_handle){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=fd_handle->object;
	handle_t* dst_fd_handle=handle_lookup_and_acquire(dst_fd,HANDLE_TYPE_FD);
	if (!dst_fd_handle){
		handle_release(fd_handle);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* dst_data=dst_fd_handle->object;
	vfs_node_t* node=vfs_get_by_id(data->node_id);
	if (!node){
		handle_release(fd_handle);
		handle_release(dst_fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	vfs_node_t* dst_node=vfs_get_by_id(dst_data->node_id);
	if (!dst_node){
		handle_release(fd_handle);
		handle_release(dst_fd_handle);
		return FD_ERROR_NOT_FOUND;
	}
	if (node->vfs_index!=dst_node->vfs_index){
		handle_release(fd_handle);
		handle_release(dst_fd_handle);
		return FD_ERROR_DIFFERENT_FS;
	}
	if (node->type!=dst_node->type){
		handle_release(fd_handle);
		handle_release(dst_fd_handle);
		return FD_ERROR_DIFFERENT_TYPE;
	}
	lock_acquire_exclusive(&(data->lock));
	lock_acquire_exclusive(&(dst_data->lock));
	_Bool error=((node->flags&VFS_NODE_TYPE_DIRECTORY)?!!vfs_get_relative(dst_node,VFS_RELATIVE_FIRST_CHILD):!!vfs_get_size(dst_node))||!vfs_move(node,dst_node);
	lock_release_exclusive(&(data->lock));
	lock_release_exclusive(&(dst_data->lock));
	handle_release(fd_handle);
	handle_release(dst_fd_handle);
	return (error?FD_ERROR_NOT_EMPTY:0);
}
