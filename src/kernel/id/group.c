#include <kernel/error/error.h>
#include <kernel/id/group.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "gid"



typedef struct _GID_DATA{
	rb_tree_node_t rb_node;
	string_t* name;
} gid_data_t;



static omm_allocator_t* _gid_data_allocator=NULL;
static rb_tree_t _gid_tree;
static spinlock_t _gid_global_lock;



KERNEL_EARLY_INIT(){
	LOG("Initializing group tree...");
	_gid_data_allocator=omm_init("gid_data",sizeof(gid_data_t),8,1,pmm_alloc_counter("omm_gid_data"));
	spinlock_init(&(_gid_data_allocator->lock));
	spinlock_init(&_gid_global_lock);
	rb_tree_init(&_gid_tree);
	INFO("Creating root group...");
	if (!gid_create(0,"root")){
		panic("Unable to create root group");
	}
}



KERNEL_PUBLIC _Bool gid_create(gid_t gid,const char* name){
	spinlock_acquire_exclusive(&_gid_global_lock);
	if (rb_tree_lookup_node(&_gid_tree,gid)){
		spinlock_release_exclusive(&_gid_global_lock);
		return 0;
	}
	gid_data_t* gid_data=omm_alloc(_gid_data_allocator);
	gid_data->rb_node.key=gid;
	gid_data->name=smm_alloc(name,0);
	rb_tree_insert_node(&_gid_tree,&(gid_data->rb_node));
	spinlock_release_exclusive(&_gid_global_lock);
	return 1;
}



KERNEL_PUBLIC _Bool gid_get_name(gid_t gid,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return 0;
	}
	spinlock_acquire_shared(&_gid_global_lock);
	gid_data_t* gid_data=(gid_data_t*)rb_tree_lookup_node(&_gid_tree,gid);
	if (!gid_data){
		spinlock_release_shared(&_gid_global_lock);
		return 0;
	}
	strcpy(buffer,gid_data->name->data,buffer_length);
	spinlock_release_shared(&_gid_global_lock);
	return 1;
}



u64 syscall_gid_get(void){
	return THREAD_DATA->process->gid;
}



u64 syscall_gid_set(u64 gid){
	if (process_is_root()){
		THREAD_DATA->process->gid=gid;
		return ERROR_OK;
	}
	return ERROR_DENIED;
}



u64 syscall_gid_get_name(u64 gid,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (buffer_length>syscall_get_user_pointer_max_length((u64)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	return (gid_get_name(gid,buffer,buffer_length)?ERROR_OK:ERROR_NOT_FOUND);
}
