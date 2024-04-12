#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "pipe"



typedef struct _PIPE_VFS_NODE{
	vfs_node_t node;
	spinlock_t lock;
	void* buffer;
	u32 write_offset;
	u32 read_offset;
	_Bool is_full;
	event_t* read_event;
	event_t* write_event;
} pipe_vfs_node_t;



static omm_allocator_t* _pipe_vfs_node_allocator=NULL;
static vfs_node_t* _pipe_root=NULL;
static KERNEL_ATOMIC u64 _pipe_next_id=0;



static vfs_node_t* _pipe_create(vfs_node_t* parent,const string_t* name,u32 flags){
	if (flags&VFS_NODE_FLAG_CREATE){
		return NULL;
	}
	pipe_vfs_node_t* out=omm_alloc(_pipe_vfs_node_allocator);
	spinlock_init(&(out->lock));
	mmap_region_t* region=mmap_alloc(process_kernel->mmap,0,PIPE_BUFFER_SIZE,MMAP_REGION_FLAG_VMM_WRITE,NULL);
	if (!region){
		panic("Unable to allocate pipe buffer");
	}
	out->buffer=(void*)(region->rb_node.key);
	out->write_offset=0;
	out->read_offset=0;
	out->is_full=0;
	out->read_event=event_create();
	out->write_event=event_create();
	return (vfs_node_t*)out;
}



static void _pipe_delete(vfs_node_t* node){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
	mmap_dealloc(process_kernel->mmap,(u64)(pipe->buffer),PIPE_BUFFER_SIZE);
	event_delete(pipe->read_event);
	event_delete(pipe->write_event);
	omm_dealloc(_pipe_vfs_node_allocator,node);
}



static u64 _pipe_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
_retry_read:
	scheduler_pause();
	spinlock_acquire_exclusive(&(pipe->lock));
	if (!pipe->is_full&&pipe->write_offset==pipe->read_offset){
		spinlock_release_exclusive(&(pipe->lock));
		if (flags&VFS_NODE_FLAG_NONBLOCKING){
			scheduler_resume();
			return 0;
		}
		event_await(pipe->read_event,1);
		goto _retry_read;
	}
	u32 max_read_size=(pipe->write_offset-pipe->read_offset)&(PIPE_BUFFER_SIZE-1);
	if (!max_read_size){
		max_read_size=PIPE_BUFFER_SIZE;
	}
	if (size>max_read_size){
		size=max_read_size;
	}
	u32 chunk_size=PIPE_BUFFER_SIZE-pipe->read_offset;
	if (chunk_size>size){
		chunk_size=size;
	}
	mem_copy(buffer,pipe->buffer+pipe->read_offset,chunk_size);
	if (size>chunk_size){
		mem_copy(buffer+chunk_size,pipe->buffer,size-chunk_size);
	}
	if (!(flags&VFS_NODE_FLAG_PIPE_PEEK)){
		pipe->read_offset=(pipe->read_offset+size)&(PIPE_BUFFER_SIZE-1);
		pipe->is_full=0;
		event_dispatch(pipe->write_event,EVENT_DISPATCH_FLAG_BYPASS_ACL);
	}
	spinlock_release_exclusive(&(pipe->lock));
	scheduler_resume();
	return size;
}



static u64 _pipe_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
_retry_write:
	scheduler_pause();
	spinlock_acquire_exclusive(&(pipe->lock));
	if (pipe->is_full){
		spinlock_release_exclusive(&(pipe->lock));
		if (flags&VFS_NODE_FLAG_NONBLOCKING){
			scheduler_resume();
			return 0;
		}
		event_await(pipe->write_event,1);
		goto _retry_write;
	}
	u32 max_write_size=(pipe->read_offset-pipe->write_offset)&(PIPE_BUFFER_SIZE-1);
	if (!max_write_size){
		max_write_size=PIPE_BUFFER_SIZE;
	}
	if (size>max_write_size){
		size=max_write_size;
	}
	u32 chunk_size=PIPE_BUFFER_SIZE-pipe->write_offset;
	if (chunk_size>size){
		chunk_size=size;
	}
	mem_copy(pipe->buffer+pipe->write_offset,buffer,chunk_size);
	if (size>chunk_size){
		mem_copy(pipe->buffer,buffer+chunk_size,size-chunk_size);
	}
	pipe->write_offset=(pipe->write_offset+size)&(PIPE_BUFFER_SIZE-1);
	pipe->is_full=(pipe->write_offset==pipe->read_offset);
	event_dispatch(pipe->read_event,EVENT_DISPATCH_FLAG_BYPASS_ACL);
	spinlock_release_exclusive(&(pipe->lock));
	scheduler_resume();
	return size;
}



static const vfs_functions_t _pipe_vfs_functions={
	_pipe_create,
	_pipe_delete,
	NULL,
	NULL,
	NULL,
	NULL,
	_pipe_read,
	_pipe_write,
	NULL,
	NULL
};



KERNEL_INIT(){
	LOG("Initializing pipes...");
	_pipe_vfs_node_allocator=omm_init("pipe_node",sizeof(pipe_vfs_node_t),8,4,pmm_alloc_counter("omm_pipe_node"));
	spinlock_init(&(_pipe_vfs_node_allocator->lock));
}



KERNEL_PUBLIC vfs_node_t* pipe_create(vfs_node_t* parent,const string_t* name){
	vfs_node_t* out;
	if (parent&&name){
		out=vfs_node_create_virtual(parent,&_pipe_vfs_functions,name);
	}
	else{
		if (!_pipe_root){
			SMM_TEMPORARY_STRING dir_name=smm_alloc("pipes",0);
			_pipe_root=vfs_node_create_virtual(vfs_lookup(NULL,"/",0,0,0),NULL,dir_name);
			_pipe_root->flags|=VFS_NODE_TYPE_DIRECTORY|(0000<<VFS_NODE_PERMISSION_SHIFT);
		}
		char buffer[64];
		SMM_TEMPORARY_STRING file_name=smm_alloc(buffer,format_string(buffer,64,"%lu:%s",__atomic_fetch_add(&_pipe_next_id,1,__ATOMIC_SEQ_CST),(name?name->data:"")));
		out=vfs_node_create_virtual(_pipe_root,&_pipe_vfs_functions,file_name);
	}
	out->flags|=VFS_NODE_TYPE_PIPE;
	return out;
}



error_t syscall_pipe_create(KERNEL_USER_POINTER const char* path){
	vfs_node_t* parent=NULL;
	SMM_TEMPORARY_STRING name_string=NULL;
	if (path){
		u64 path_length=syscall_get_string_length((const char*)path);
		if (!path_length||path_length>4095){
			return ERROR_INVALID_ARGUMENT(0);
		}
		char buffer[4096];
		mem_copy(buffer,(const char*)path,path_length);
		buffer[path_length]=0;
		const char* name;
		if (vfs_lookup_for_creation(NULL,buffer,0,0,0,&parent,&name)){
			return ERROR_ALREADY_PRESENT;
		}
		if (!parent||!name){
			return ERROR_NOT_FOUND;
		}
		name_string=smm_alloc(name,0);
	}
	vfs_node_t* node=pipe_create(parent,name_string);
	node->flags|=0660<<VFS_NODE_PERMISSION_SHIFT;
	node->uid=THREAD_DATA->process->uid;
	node->gid=THREAD_DATA->process->gid;
	return fd_from_node(node,FD_FLAG_READ|FD_FLAG_WRITE);
}
