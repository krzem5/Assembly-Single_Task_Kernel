#include <kernel/fd/fd.h>
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
#define KERNEL_LOG_NAME ""



PMM_DECLARE_COUNTER(OMM_);
PMM_DECLARE_COUNTER(OMM__ITER);



static omm_allocator_t __allocator=OMM_ALLOCATOR_INIT_STRUCT("",sizeof(_t),8,4,PMM_COUNTER_OMM_);
static omm_allocator_t __iterator_allocator=OMM_ALLOCATOR_INIT_STRUCT("_iterator",sizeof(_iterator_t),8,4,PMM_COUNTER_OMM__ITER);



static HANDLE_DECLARE_TYPE(,{
	_t* data=handle->object;
	omm_dealloc(&__allocator,data);
});
static HANDLE_DECLARE_TYPE(_ITERATOR,{
	_iterator_t* data=handle->object;
	omm_dealloc(&__iterator_allocator,data);
});



static handle_id_t _node_to_fd(vfs2_node_t* node,u32 flags){
	_t* out=omm_alloc(&__allocator);
	handle_new(out,HANDLE_TYPE_,&(out->handle));
	lock_init(&(out->lock));
	out->node=node;
	out->offset=((flags&_FLAG_APPEND)?vfs2_node_resize(node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE):0);
	out->flags=flags&(_FLAG_READ|_FLAG_WRITE);
	return out->handle.rb_node.key;
}



s64 _open(handle_id_t root,const char* path,u32 length,u32 flags){
	if (flags&(~(_FLAG_READ|_FLAG_WRITE|_FLAG_APPEND|_FLAG_CREATE|_FLAG_DIRECTORY))){
		return _ERROR_INVALID_FLAGS;
	}
	char buffer[4096];
	if (length>4095){
		return _ERROR_INVALID_POINTER;
	}
	memcpy(buffer,path,length);
	buffer[length]=0;
	handle_t* root_handle=NULL;
	vfs2_node_t* root_node=NULL;
	if (root){
		root_handle=handle_lookup_and_acquire(root,HANDLE_TYPE_);
		if (!root_handle){
			return _ERROR_INVALID_FD;
		}
		root_node=((_t*)(root_handle->object))->node;
	}
	if (flags&_FLAG_CREATE){
		panic("_FLAG_CREATE");
	}
	vfs2_node_t* node=vfs2_lookup(root_node,buffer);
	if (root_handle){
		handle_release(root_handle);
	}
	if (!node){
		return _ERROR_NOT_FOUND;
	}
	return _node_to_fd(node,flags);
}



s64 _close(handle_id_t fd){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	handle_release(_handle);
	handle_release(_handle);
	return 0;
}



s64 _read(handle_id_t fd,void* buffer,u64 count){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	_t* data=_handle->object;
	if (!(data->flags&_FLAG_READ)){
		handle_release(_handle);
		return _ERROR_UNSUPPORTED_OPERATION;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs2_node_read(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(_handle);
	return count;
}



s64 _write(handle_id_t fd,const void* buffer,u64 count){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	_t* data=_handle->object;
	if (!(data->flags&_FLAG_WRITE)){
		handle_release(_handle);
		return _ERROR_UNSUPPORTED_OPERATION;
	}
	lock_acquire_exclusive(&(data->lock));
	count=vfs2_node_write(data->node,data->offset,buffer,count);
	data->offset+=count;
	lock_release_exclusive(&(data->lock));
	handle_release(_handle);
	return count;
}



s64 _seek(handle_id_t fd,u64 offset,u32 type){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	_t* data=_handle->object;
	lock_acquire_exclusive(&(data->lock));
	switch (type){
		case _SEEK_SET:
			data->offset=offset;
			break;
		case _SEEK_ADD:
			data->offset+=offset;
			break;
		case _SEEK_END:
			data->offset=vfs2_node_resize(data->node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE);
			break;
		default:
			lock_release_exclusive(&(data->lock));
			handle_release(_handle);
			return _ERROR_INVALID_FLAGS;
	}
	u64 out=data->offset;
	lock_release_exclusive(&(data->lock));
	handle_release(_handle);
	return out;
}



s64 _resize(handle_id_t fd,u64 size,u32 flags){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	_t* data=_handle->object;
	lock_acquire_exclusive(&(data->lock));
	s64 out=(vfs2_node_resize(data->node,size,0)?0:_ERROR_NO_SPACE);
	if (!out&&data->offset>size){
		data->offset=size;
	}
	lock_release_exclusive(&(data->lock));
	handle_release(_handle);
	return out;
}



s64 _stat(handle_id_t fd,_stat_t* out){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	_t* data=_handle->object;
	lock_acquire_exclusive(&(data->lock));
	out->type=data->node->flags&VFS2_NODE_TYPE_MASK;
	out->vfs_index=/*data->node->vfs_index*/0xaa;
	out->name_length=data->node->name->length;
	memcpy(out->name,data->node->name->data,64);
	out->size=vfs2_node_resize(data->node,0,VFS2_NODE_FLAG_RESIZE_RELATIVE);
	lock_release_exclusive(&(data->lock));
	handle_release(_handle);
	return 0;
}



s64 _dup(handle_id_t fd,u32 flags){
	panic("_dup");
}



s64 _path(handle_id_t fd,char* buffer,u32 buffer_length){
	panic("_path");
}



s64 _iter_start(handle_id_t fd){
	handle_t* _handle=handle_lookup_and_acquire(fd,HANDLE_TYPE_);
	if (!_handle){
		return _ERROR_INVALID_FD;
	}
	_t* data=_handle->object;
	lock_acquire_exclusive(&(data->lock));
	vfs2_name_t* current_name;
	u64 pointer=vfs2_node_iterate(data->node,0,&current_name);
	if (!pointer){
		lock_release_exclusive(&(data->lock));
		handle_release(_handle);
		return -1;
	}
	_iterator_t* out=omm_alloc(&__iterator_allocator);
	handle_new(out,HANDLE_TYPE__ITERATOR,&(out->handle));
	lock_init(&(out->lock));
	out->node=data->node;
	out->pointer=pointer;
	out->current_name=current_name;
	lock_release_exclusive(&(data->lock));
	handle_release(_handle);
	return out->handle.rb_node.key;
}



s64 _iter_get(handle_id_t iterator,char* buffer,u32 buffer_length){
	handle_t* _iterator_handle=handle_lookup_and_acquire(iterator,HANDLE_TYPE__ITERATOR);
	if (!_iterator_handle){
		return _ERROR_INVALID_FD;
	}
	_iterator_t* data=_iterator_handle->object;
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
	handle_release(_iterator_handle);
	return buffer_length;
}



s64 _iter_next(handle_id_t iterator){
	handle_t* _iterator_handle=handle_lookup_and_acquire(iterator,HANDLE_TYPE__ITERATOR);
	if (!_iterator_handle){
		return _ERROR_INVALID_FD;
	}
	_iterator_t* data=_iterator_handle->object;
	lock_acquire_exclusive(&(data->lock));
	s64 out=-1;
	if (data->current_name){
		vfs2_name_dealloc(data->current_name);
		data->pointer=vfs2_node_iterate(data->node,data->pointer,&(data->current_name));
		if (!data->pointer){
			handle_release(_iterator_handle);
		}
		else{
			out=_iterator_handle->rb_node.key;
		}
	}
	lock_release_exclusive(&(data->lock));
	handle_release(_iterator_handle);
	return out;
}



s64 _iter_stop(handle_id_t iterator){
	panic("_iter_stop");
}
