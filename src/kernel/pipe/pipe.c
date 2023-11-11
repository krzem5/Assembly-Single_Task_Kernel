#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/notification/notification.h>
#include <kernel/pipe/pipe.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
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



static pmm_counter_descriptor_t _pipe_buffer_pmm_counter=PMM_COUNTER_INIT_STRUCT("pipe_buffer");
static pmm_counter_descriptor_t _pipe_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_pipe_node");
static omm_allocator_t _pipe_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("pipe_node",sizeof(pipe_vfs_node_t),8,4,&_pipe_node_omm_pmm_counter);



static notification_dispatcher_t _pipe_notification_dispatcher=NOTIFICATION_DISPATCHER_INIT_STRUCT;



static vfs_node_t* _pipe_create(void){
	pipe_vfs_node_t* out=omm_alloc(&_pipe_vfs_node_allocator);
	spinlock_init(&(out->lock));
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,PIPE_BUFFER_SIZE,&_pipe_buffer_pmm_counter,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_COMMIT,NULL);
	if (!region){
		panic("Unable to allocate pipe buffer");
	}
	out->buffer=(void*)(region->rb_node.key);
	out->write_offset=0;
	out->read_offset=0;
	out->is_full=0;
	out->read_event=event_new();
	out->write_event=event_new();
	return (vfs_node_t*)out;
}



static u64 _pipe_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	pipe_vfs_node_t* pipe=(pipe_vfs_node_t*)node;
_retry_read:
	scheduler_pause();
	spinlock_acquire_exclusive(&(pipe->lock));
	if (!pipe->is_full&&pipe->write_offset==pipe->read_offset){
		spinlock_release_exclusive(&(pipe->lock));
		if (flags&VFS_NODE_FLAG_NONBLOCKING){
			return 0;
		}
		thread_await_event(pipe->write_event);
		goto _retry_read;
	}
	u32 max_read_size=(pipe->read_offset-pipe->write_offset)&(PIPE_BUFFER_SIZE-1);
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
	memcpy(buffer,pipe->buffer+pipe->read_offset,chunk_size);
	if (size>chunk_size){
		memcpy(buffer+chunk_size,pipe->buffer,size-chunk_size);
	}
	pipe->read_offset=(pipe->read_offset+size)&(PIPE_BUFFER_SIZE-1);
	pipe->is_full=0;
	event_dispatch(pipe->read_event,1);
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
			return 0;
		}
		thread_await_event(pipe->read_event);
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
	memcpy(pipe->buffer+pipe->write_offset,buffer,chunk_size);
	if (size>chunk_size){
		memcpy(pipe->buffer,buffer+chunk_size,size-chunk_size);
	}
	pipe->write_offset=(pipe->write_offset+size)&(PIPE_BUFFER_SIZE-1);
	pipe->is_full=(pipe->write_offset==pipe->read_offset);
	event_dispatch(pipe->write_event,1);
	spinlock_release_exclusive(&(pipe->lock));
	scheduler_resume();
	return size;
}



static const vfs_functions_t _pipe_vfs_functions={
	_pipe_create,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	_pipe_read,
	_pipe_write,
	NULL,
	NULL
};



vfs_node_t* pipe_create(vfs_node_t* parent,const string_t* name){
	vfs_node_t* out=vfs_node_create_virtual(parent,&_pipe_vfs_functions,name);
	out->flags|=VFS_NODE_TYPE_PIPE;
	return out;
}



void pipe_register_notification_listener(notification_listener_t* listener){
	notification_dispatcher_add_listener(&_pipe_notification_dispatcher,listener);
}



void pipe_unregister_notification_listener(notification_listener_t* listener){
	notification_dispatcher_remove_listener(&_pipe_notification_dispatcher,listener);
}
