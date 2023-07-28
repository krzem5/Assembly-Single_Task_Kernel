#include <kernel/fs/fd.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/node_allocator.h>
#include <kernel/log/log.h>
#include <kernel/lock/lock.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



static lock_t _fd_lock=LOCK_INIT_STRUCT;
static fd_data_t* _fd_data;
static fd_t _fd_count;
static u64 _fd_bitmap[FD_COUNT];



static inline _Bool _is_invalid_fd(fd_t fd){
	return (!fd||fd>FD_COUNT||(_fd_bitmap[(fd-1)>>6]&(1ull<<((fd-1)&63))));
}



static inline fd_data_t* _get_fd_data(fd_t fd){
	return _fd_data+fd-1;
}



static int _node_to_fd(fs_node_t* node,u8 flags){
	lock_acquire(&_fd_lock);
	if (_fd_count>=FD_COUNT){
		return FD_ERROR_OUT_OF_FDS;
	}
	fd_t out=0;
	while (!_fd_bitmap[out]){
		out++;
	}
	fd_t idx=__builtin_ctzll(_fd_bitmap[out]);
	_fd_bitmap[out]&=_fd_bitmap[out]-1;
	out=(out<<6)|idx;
	fd_data_t* data=_fd_data+out;
	data->node_id=node->id;
	data->offset=((flags&FD_FLAG_APPEND)?fs_get_size(node):0);
	data->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE);
	lock_release(&_fd_lock);
	return idx+1;
}



void fd_init(void){
	LOG("Initializing file descriptor list...");
	_fd_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(FD_COUNT*sizeof(fd_data_t))));
}



void fd_clear(void){
	LOG("Clearing file descriptor list...");
	lock_acquire(&_fd_lock);
	_fd_count=0;
	for (u16 i=0;i<((FD_COUNT+63)>>6);i++){
		_fd_bitmap[i]=0xffffffffffffffffull;
	}
	lock_release(&_fd_lock);
}



int fd_open(fd_t root,const char* path,u32 length,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY))){
		return FD_ERROR_INVALID_FLAGS;
	}
	char buffer[4096];
	if (length>4095){
		return FD_ERROR_INVALID_POINTER;
	}
	memcpy(buffer,path,length);
	lock_acquire(&_fd_lock);
	fs_node_t* root_node=NULL;
	if (root){
		if (_is_invalid_fd(root)){
			lock_release(&_fd_lock);
			return FD_ERROR_INVALID_FD;
		}
		root_node=fs_get_node_by_id(_get_fd_data(root)->node_id);
		if (!root_node){
			lock_release(&_fd_lock);
			return FD_ERROR_NOT_FOUND;
		}
	}
	buffer[length]=0;
	fs_node_t* node=fs_get_node(root_node,buffer,((flags&FD_FLAG_CREATE)?((flags&FD_FLAG_DIRECTORY)?FS_NODE_TYPE_DIRECTORY:FS_NODE_TYPE_FILE):0));
	lock_release(&_fd_lock);
	if (!node){
		return FD_ERROR_NOT_FOUND;
	}
	return _node_to_fd(node,flags);
}



int fd_close(fd_t fd){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	_fd_bitmap[(fd-1)>>6]|=1ull<<((fd-1)&63);
	lock_release(&_fd_lock);
	return 0;
}



int fd_delete(fd_t fd){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	fs_node_t* node=fs_get_node_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	if (node->type==FS_NODE_TYPE_DIRECTORY&&fs_get_node_relative(node,FS_NODE_RELATIVE_FIRST_CHILD)){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_EMPTY;
	}
	fs_node_t* prev=fs_get_node_relative(node,FS_NODE_RELATIVE_PREV_SIBLING);
	fs_node_t* next=fs_get_node_relative(node,FS_NODE_RELATIVE_NEXT_SIBLING);
	_Bool out=1;
	if (prev){
		out&=fs_set_node_relative(prev,FS_NODE_RELATIVE_NEXT_SIBLING,next);
	}
	else{
		out&=fs_set_node_relative(fs_get_node_relative(node,FS_NODE_RELATIVE_PARENT),FS_NODE_RELATIVE_FIRST_CHILD,next);
	}
	if (next){
		out&=fs_set_node_relative(next,FS_NODE_RELATIVE_PREV_SIBLING,prev);
	}
	if (out){
		out=fs_dealloc_node(node);
	}
	if (out){
		_fd_bitmap[(fd-1)>>6]|=1ull<<((fd-1)&63);
	}
	lock_release(&_fd_lock);
	return (out?0:FD_ERROR_NOT_EMPTY);
}



s64 fd_read(fd_t fd,void* buffer,u64 count){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	if (!(data->flags&FD_FLAG_READ)){
		lock_release(&_fd_lock);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	fs_node_t* node=fs_get_node_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	count=fs_read(node,data->offset,buffer,count);
	data->offset+=count;
	lock_release(&_fd_lock);
	return count;
}



s64 fd_write(fd_t fd,const void* buffer,u64 count){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	if (!(data->flags&FD_FLAG_WRITE)){
		lock_release(&_fd_lock);
		return FD_ERROR_UNSUPPORTED_OPERATION;
	}
	fs_node_t* node=fs_get_node_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	count=fs_write(node,data->offset,buffer,count);
	data->offset+=count;
	lock_release(&_fd_lock);
	return count;
}



s64 fd_seek(fd_t fd,u64 offset,u8 flags){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	switch (flags){
		case FD_SEEK_SET:
			data->offset=offset;
			break;
		case FD_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD_SEEK_END:
			fs_node_t* node=fs_get_node_by_id(data->node_id);
			if (!node){
				lock_release(&_fd_lock);
				return FD_ERROR_NOT_FOUND;
			}
			data->offset=fs_get_size(node);
			break;
		default:
			lock_release(&_fd_lock);
			return FD_ERROR_INVALID_FLAGS;
	}
	lock_release(&_fd_lock);
	return data->offset;
}



int fd_stat(fd_t fd,fd_stat_t* out){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	fs_node_t* node=fs_get_node_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	out->node_id=node->id;
	out->type=node->type;
	out->fs_index=node->fs_index;
	out->name_length=node->name_length;
	memcpy(out->name,node->name,64);
	out->size=fs_get_size(node);
	lock_release(&_fd_lock);
	return 0;
}



int fd_get_relative(fd_t fd,u8 relative,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND))){
		return FD_ERROR_INVALID_FLAGS;
	}
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_get_fd_data(fd);
	fs_node_t* node=fs_get_node_by_id(data->node_id);
	if (!node){
		lock_release(&_fd_lock);
		return FD_ERROR_NOT_FOUND;
	}
	fs_node_t* other=fs_get_node_relative(node,relative);
	lock_release(&_fd_lock);
	if (!other){
		return FD_ERROR_NO_RELATIVE;
	}
	return _node_to_fd(other,flags);
}



int fd_dup(fd_t fd,u8 flags){
	lock_acquire(&_fd_lock);
	if (_is_invalid_fd(fd)){
		lock_release(&_fd_lock);
		return FD_ERROR_INVALID_FD;
	}
	WARN("Unimplemented: fd_dup");
	lock_release(&_fd_lock);
	return -1;
}
