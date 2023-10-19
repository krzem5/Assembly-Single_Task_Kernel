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
#define KERNEL_LOG_NAME "fd2"



PMM_DECLARE_COUNTER(OMM_FD2);
PMM_DECLARE_COUNTER(OMM_FD2_ITER);



static omm_allocator_t _fd2_allocator=OMM_ALLOCATOR_INIT_STRUCT("fd2",sizeof(fd2_t),8,4,PMM_COUNTER_OMM_FD2);
static omm_allocator_t _fd2_iterator_allocator=OMM_ALLOCATOR_INIT_STRUCT("fd2_iterator",sizeof(fd2_iterator_t),8,4,PMM_COUNTER_OMM_FD2_ITER);



static HANDLE_DECLARE_TYPE(FD2,{
	fd2_t* data=handle->object;
	omm_dealloc(&_fd2_allocator,data);
});
static HANDLE_DECLARE_TYPE(FD2_ITERATOR,{
	fd2_iterator_t* data=handle->object;
	omm_dealloc(&_fd2_iterator_allocator,data);
});



static handle_id_t _node_to_fd(vfs2_node_t* node,u32 flags){
	fd2_t* out=omm_alloc(&_fd2_allocator);
	handle_new(out,HANDLE_TYPE_FD2,&(out->handle));
	lock_init(&(out->lock));
	out->node=node;
	out->offset=((flags&FD2_FLAG_APPEND)?vfs2_node_resize(node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE):0);
	out->flags=flags&(FD2_FLAG_READ|FD2_FLAG_WRITE);
	return out->handle.rb_node.key;
}



s64 fd2_open(handle_id_t root,const char* path,u32 length,u32 flags){
	if (flags&(~(FD2_FLAG_READ|FD2_FLAG_WRITE|FD2_FLAG_APPEND|FD2_FLAG_CREATE|FD2_FLAG_DIRECTORY))){
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
	lock_acquire_exclusive(&(data->lock));
	count=vfs2_node_write(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return count;
}



s64 fd2_seek(handle_id_t fd,u64 offset,u32 type){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	lock_acquire_exclusive(&(data->lock));
	switch (type){
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



s64 fd2_resize(handle_id_t fd,u64 size,u32 flags){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	lock_acquire_exclusive(&(data->lock));
	s64 out=(vfs2_node_resize(data->node,size,0)?0:FD2_ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	return out;
}



s64 fd2_stat(handle_id_t fd,fd2_stat_t* out){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
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



s64 fd2_dup(handle_id_t fd,u32 flags){
	panic("fd2_dup");
}



s64 fd2_path(handle_id_t fd,char* buffer,u32 buffer_length){
	panic("fd2_path");
}



s64 fd2_iter_start(handle_id_t fd){
	handle_t* fd2_handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_FD2);
	if (!fd2_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_t* data=fd2_handle->object;
	lock_acquire_exclusive(&(data->lock));
	vfs2_name_t* current_name;
	u64 pointer=vfs2_node_iterate(data->node,0,&current_name);
	LOG("~~~ %u",__LINE__);
	if (!pointer){
		lock_release_exclusive(&(data->lock));
		handle_release(fd2_handle);
		return -1;
	}
	LOG("~~~ %u",__LINE__);
	fd2_iterator_t* out=omm_alloc(&_fd2_iterator_allocator);
	handle_new(out,HANDLE_TYPE_FD2_ITERATOR,&(out->handle));
	lock_init(&(out->lock));
	out->node=data->node;
	out->pointer=pointer;
	out->current_name=current_name;
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_handle);
	LOG("~~~ %u %p",__LINE__,out->handle.rb_node.key);
	return out->handle.rb_node.key;
}



s64 fd2_iter_get(handle_id_t iterator,char* buffer,u32 buffer_length){
	handle_t* fd2_iterator_handle=handle_lookup_and_acquire(iterator,HANDLE_TYPE_FD2_ITERATOR);
	if (!fd2_iterator_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_iterator_t* data=fd2_iterator_handle->object;
	lock_acquire_shared(&(data->lock));
	if (data->current_name){
		if (buffer_length>data->current_name->length+1){
			buffer_length=data->current_name->length+1;
		}
		if (buffer_length){
			memcpy(buffer,data->current_name->data,buffer_length-1);
			buffer[buffer_length]=0;
		}
	}
	else{
		buffer_length=0;
	}
	lock_release_shared(&(data->lock));
	handle_release(fd2_iterator_handle);
	return buffer_length;
}



s64 fd2_iter_next(handle_id_t iterator){
	handle_t* fd2_iterator_handle=handle_lookup_and_acquire(iterator,HANDLE_TYPE_FD2_ITERATOR);
	if (!fd2_iterator_handle){
		return FD2_ERROR_INVALID_FD;
	}
	fd2_iterator_t* data=fd2_iterator_handle->object;
	lock_acquire_exclusive(&(data->lock));
	s64 out=-1;
	if (data->current_name){
		vfs2_name_dealloc(data->current_name);
		data->pointer=vfs2_node_iterate(data->node,data->pointer,&(data->current_name));
		if (!data->pointer){
			handle_release(fd2_iterator_handle);
		}
		else{
			out=fd2_iterator_handle->rb_node.key;
		}
	}
	lock_release_exclusive(&(data->lock));
	handle_release(fd2_iterator_handle);
	return out;
}



s64 fd2_iter_stop(handle_id_t iterator){
	panic("fd2_iter_stop");
}
