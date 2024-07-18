#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "pipe"



typedef struct _PIPE_VFS_NODE{
	vfs_node_t node;
	rwlock_t lock;
	void* buffer;
	u32 write_offset;
	u32 read_offset;
	bool is_full;
	bool is_closed;
	event_t* read_event;
	event_t* write_event;
} pipe_vfs_node_t;



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _pipe_buffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _pipe_vfs_node_allocator=NULL;
static vfs_node_t* _pipe_root=NULL;
static KERNEL_ATOMIC u64 _pipe_next_id=0;



static vfs_node_t* _pipe_create(vfs_node_t* parent,const string_t* name,u32 flags){
	if (flags&VFS_NODE_FLAG_CREATE){
		return NULL;
	}
	pipe_vfs_node_t* out=omm_alloc(_pipe_vfs_node_allocator);
	rwlock_init(&(out->lock));
	out->buffer=(void*)(pmm_alloc(pmm_align_up_address(PIPE_BUFFER_SIZE)>>PAGE_SIZE_SHIFT,_pipe_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	out->write_offset=0;
	out->read_offset=0;
	out->is_full=0;
	out->is_closed=0;
	out->read_event=event_create("kernel.pipe.read",NULL);
	out->write_event=event_create("kernel.pipe.write",NULL);
	event_set_active(out->write_event,1,1);
	return (vfs_node_t*)out;
}



static void _pipe_delete(vfs_node_t* node){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
	pmm_dealloc(((u64)(pipe->buffer))-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(PIPE_BUFFER_SIZE)>>PAGE_SIZE_SHIFT,_pipe_buffer_pmm_counter);
	event_delete(pipe->read_event);
	event_delete(pipe->write_event);
	omm_dealloc(_pipe_vfs_node_allocator,node);
}



static KERNEL_AWAITS u64 _pipe_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
_retry_read:
	rwlock_acquire_write(&(pipe->lock));
	if (!pipe->is_full&&pipe->write_offset==pipe->read_offset){
		if (pipe->is_closed){
			rwlock_release_write(&(pipe->lock));
			return 0;
		}
		rwlock_release_write(&(pipe->lock));
		if (flags&VFS_NODE_FLAG_NONBLOCKING){
			return 0;
		}
		event_await(&(pipe->read_event),1,1);
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
		if (pipe->write_offset==pipe->read_offset){
			event_set_active(pipe->read_event,0,1);
		}
		pipe->is_full=0;
		event_dispatch(pipe->write_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	}
	rwlock_release_write(&(pipe->lock));
	return size;
}



static KERNEL_AWAITS u64 _pipe_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
_retry_write:
	rwlock_acquire_write(&(pipe->lock));
	if (pipe->is_closed){
		rwlock_release_write(&(pipe->lock));
		return 0;
	}
	if (pipe->is_full){
		rwlock_release_write(&(pipe->lock));
		if (flags&VFS_NODE_FLAG_NONBLOCKING){
			return 0;
		}
		event_await(&(pipe->write_event),1,1);
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
	if (pipe->is_full){
		event_set_active(pipe->write_event,0,1);
	}
	event_dispatch(pipe->read_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	rwlock_release_write(&(pipe->lock));
	return size;
}



static event_t* _pipe_event(vfs_node_t* node,bool is_write){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
	return (is_write?pipe->write_event:pipe->read_event);
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
	NULL,
	_pipe_event,
};



KERNEL_INIT(){
	LOG("Initializing pipes...");
	_pipe_buffer_pmm_counter=pmm_alloc_counter("kernel.pipe.buffer");
	_pipe_vfs_node_allocator=omm_init("kernel.pipe.vfs_node",sizeof(pipe_vfs_node_t),8,4);
	rwlock_init(&(_pipe_vfs_node_allocator->lock));
}



KERNEL_PUBLIC KERNEL_NO_AWAITS vfs_node_t* pipe_create(vfs_node_t* parent,const string_t* name){
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



KERNEL_PUBLIC error_t pipe_close(vfs_node_t* node){
	if (node->functions!=&_pipe_vfs_functions){
		return ERROR_UNSUPPORTED_OPERATION;
	}
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
	rwlock_acquire_write(&(pipe->lock));
	bool out=!pipe->is_closed;
	pipe->is_closed=1;
	event_set_active(pipe->read_event,1,1);
	event_set_active(pipe->write_event,1,1);
	rwlock_release_write(&(pipe->lock));
	return (out?ERROR_OK:ERROR_FAILED);
}



KERNEL_AWAITS error_t syscall_pipe_create(KERNEL_USER_POINTER const char* path){
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
			if (parent){
				vfs_node_unref(parent);
			}
			return ERROR_NOT_FOUND;
		}
		name_string=smm_alloc(name,0);
	}
	vfs_node_t* node=pipe_create(parent,name_string);
	if (parent){
		vfs_node_unref(parent);
	}
	node->flags|=0660<<VFS_NODE_PERMISSION_SHIFT;
	node->uid=THREAD_DATA->process->uid;
	node->gid=THREAD_DATA->process->gid;
	if (!path){
		node->flags|=VFS_NODE_FLAG_TEMPORARY;
	}
	error_t out=fd_from_node(node,FD_FLAG_READ|FD_FLAG_WRITE);
	vfs_node_unref(node);
	return out;
}



error_t syscall_pipe_close(handle_id_t fd){
	u64 acl;
	vfs_node_t* pipe=fd_get_node(fd,&acl);
	if (!pipe){
		return ERROR_INVALID_HANDLE;
	}
	if (!(acl&FD_ACL_FLAG_CLOSE)){
		vfs_node_unref(pipe);
		return ERROR_DENIED;
	}
	error_t out=pipe_close(pipe);
	vfs_node_unref(pipe);
	return out;
}
