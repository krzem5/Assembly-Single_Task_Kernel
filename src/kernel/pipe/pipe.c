#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/process.h>
#include <kernel/notification/notification.h>
#include <kernel/pipe/pipe.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>



typedef struct _PIPE_VFS_NODE{
	vfs_node_t node;
	spinlock_t lock;
	void* buffer;
	u32 write_offset;
	u32 read_offset;
	_Bool is_full;
} pipe_vfs_node_t;



static pmm_counter_descriptor_t _pipe_buffer_pmm_counter=PMM_COUNTER_INIT_STRUCT("pipe_buffer");
static pmm_counter_descriptor_t _pipe_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_pipe_node");
static omm_allocator_t _pipe_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("pipe_node",sizeof(pipe_vfs_node_t),8,4,&_pipe_node_omm_pmm_counter);



static notification_dispatcher_t _pipe_notification_dispatcher=NOTIFICATION_DISPATCHER_INIT_STRUCT;



static vfs_node_t* _pipe_create(void){
	pipe_vfs_node_t* out=omm_alloc(&_pipe_vfs_node_allocator);
	spinlock_init(&(out->lock));
	mmap_region_t* region=mmap_alloc(&(process_kernel->mmap),0,PIPE_BUFFER_PAGE_COUNT<<PAGE_SIZE_SHIFT,&_pipe_buffer_pmm_counter,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
	if (!region){
		panic("Unable to allocate pipe buffer");
	}
	out->buffer=(void*)(region->rb_node.key);
	out->write_offset=0;
	out->read_offset=0;
	out->is_full=0;
	return (vfs_node_t*)out;
}



static const vfs_functions_t _pipe_vfs_functions={
	_pipe_create,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};



vfs_node_t* pipe_create(vfs_node_t* parent,const string_t* name){
	return vfs_node_create_virtual(parent,&_pipe_vfs_functions,name);
}



void pipe_register_notification_listener(notification_listener_t* listener){
	notification_dispatcher_add_listener(&_pipe_notification_dispatcher,listener);
}



void pipe_unregister_notification_listener(notification_listener_t* listener){
	notification_dispatcher_remove_listener(&_pipe_notification_dispatcher,listener);
}
