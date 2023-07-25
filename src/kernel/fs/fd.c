#include <kernel/fs/fd.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/node_allocator.h>
#include <kernel/log/log.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



static fd_data_t* _fd_data;
static fd_t _fd_count;
static u64 _fd_bitmap[FD_COUNT];



static inline _Bool _is_invalid_fd(fd_t fd){
	return (fd>=FD_COUNT||(_fd_bitmap[fd>>6]&(1ull<<(fd&63))));
}



// static fd_data_t* _get_fd_data(fd_t fd){
// 	if (fd>=_fd_count||(_fd_data+fd)->node_id==FS_NODE_ID_EMPTY){
// 		return NULL;
// 	}
// 	return _fd_data+fd;
// }



void fd_init(void){
	LOG("Initializing file descriptor list...");
	_fd_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(FD_COUNT*sizeof(fd_data_t))));
}



void fd_clear(void){
	LOG("Clearing file descriptor list...");
	_fd_count=0;
	for (u16 i=0;i<((FD_COUNT+63)>>6);i++){
		_fd_bitmap[i]=0xffffffffffffffffull;
	}
}



int fd_open(const char* path,u32 length,u8 flags){
	if (flags&(~(FD_FLAG_READ|FD_FLAG_WRITE|FD_FLAG_APPEND|FD_FLAG_CREATE|FD_FLAG_DIRECTORY))){
		return FD_ERROR_INVALID_FLAGS;
	}
	if (_fd_count>=FD_COUNT){
		return FD_ERROR_OUT_OF_FDS;
	}
	char buffer[4096];
	if (length>4095){
		return FD_ERROR_INVALID_POINTER;
	}
	memcpy(buffer,path,length);
	buffer[length]=0;
	fs_node_t* node=fs_get_node(NULL,buffer);
	if (!node){
		return FD_ERROR_NOT_FOUND;
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
	data->offset=0;
	data->flags=flags&(FD_FLAG_READ|FD_FLAG_WRITE);
	if (flags&FD_FLAG_APPEND){
		ERROR("Unimplemented: set offset of fd to file size");
		for (;;);
	}
	return idx;
}



int fd_close(fd_t fd){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	_fd_bitmap[fd>>6]|=1ull<<(fd&63);
	return 0;
}



int fd_delete(fd_t fd){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	WARN("Unimplemented: fd_delete");
	return -1;
}



s64 fd_read(fd_t fd,void* buffer,u64 count){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	WARN("Unimplemented: fd_read");
	return -1;
}



s64 fd_write(fd_t fd,const void* buffer,u64 count){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_fd_data+fd;
	fs_node_t* node=fs_get_node_by_id(1,data->node_id);
	if (!node){
		return FD_ERROR_NOT_FOUND;
	}
	count=fs_write(node,data->offset,buffer,count);
	data->offset+=count;
	return count;
}



s64 fd_seek(fd_t fd,u64 offset,u8 flags){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	fd_data_t* data=_fd_data+fd;
	switch (flags){
		case FD_SEEK_SET:
			data->offset=offset;
			break;
		case FD_SEEK_ADD:
			data->offset+=offset;
			break;
		case FD_SEEK_END:
			WARN("Unimplemented: FD_SEEK_END");
			return -1;
		default:
			return FD_ERROR_INVALID_FLAGS;
	}
	return data->offset;
}



int fd_get_relative(fd_t fd,u8 relative){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	WARN("Unimplemented: fd_get_relative");
	return -1;
}



int fd_dup(fd_t fd,u8 flags){
	if (_is_invalid_fd(fd)){
		return FD_ERROR_INVALID_FD;
	}
	WARN("Unimplemented: fd_dup");
	return -1;
}
