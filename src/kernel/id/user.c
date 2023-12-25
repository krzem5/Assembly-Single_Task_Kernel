#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "uid"



typedef struct _UID_GROUP{
	rb_tree_node_t rb_node;
} uid_group_t;



typedef struct _UID_DATA{
	rb_tree_node_t rb_node;
	string_t* name;
	rb_tree_t group_tree;
} uid_data_t;



static omm_allocator_t* _uid_data_allocator=NULL;
static omm_allocator_t* _uid_group_allocator=NULL;
static rb_tree_t _uid_tree;
static spinlock_t _uid_global_lock;



KERNEL_EARLY_INIT(){
	LOG("Initializing user tree...");
	_uid_data_allocator=omm_init("uid_data",sizeof(uid_data_t),8,1,pmm_alloc_counter("omm_uid_data"));
	spinlock_init(&(_uid_data_allocator->lock));
	_uid_group_allocator=omm_init("uid_group",sizeof(uid_group_t),8,1,pmm_alloc_counter("omm_uid_group"));
	spinlock_init(&(_uid_group_allocator->lock));
	spinlock_init(&_uid_global_lock);
	rb_tree_init(&_uid_tree);
	INFO("Creating root user...");
	if (!uid_create(0,"root")||!uid_add_group(0,0)){
		panic("Unable to create root user");
	}
}



KERNEL_PUBLIC _Bool uid_create(uid_t uid,const char* name){
	spinlock_acquire_exclusive(&_uid_global_lock);
	if (rb_tree_lookup_node(&_uid_tree,uid)){
		spinlock_release_exclusive(&_uid_global_lock);
		return 0;
	}
	uid_data_t* uid_data=omm_alloc(_uid_data_allocator);
	uid_data->rb_node.key=uid;
	uid_data->name=smm_alloc(name,0);
	rb_tree_init(&(uid_data->group_tree));
	rb_tree_insert_node(&_uid_tree,&(uid_data->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	return 1;
}



KERNEL_PUBLIC _Bool uid_add_group(uid_t uid,gid_t gid){
	spinlock_acquire_exclusive(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data||rb_tree_lookup_node(&(uid_data->group_tree),gid)){
		spinlock_release_exclusive(&_uid_global_lock);
		return 0;
	}
	uid_group_t* uid_group=omm_alloc(_uid_group_allocator);
	uid_group->rb_node.key=gid;
	rb_tree_insert_node(&(uid_data->group_tree),&(uid_group->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	return 1;
}



KERNEL_PUBLIC _Bool uid_has_group(uid_t uid,gid_t gid){
	spinlock_acquire_shared(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	_Bool out=(uid_data&&rb_tree_lookup_node(&(uid_data->group_tree),gid));
	spinlock_release_shared(&_uid_global_lock);
	return out;
}



KERNEL_PUBLIC _Bool uid_get_name(uid_t uid,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return 0;
	}
	spinlock_acquire_shared(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data){
		spinlock_release_shared(&_uid_global_lock);
		return 0;
	}
	strcpy(buffer,uid_data->name->data,buffer_length);
	spinlock_release_shared(&_uid_global_lock);
	return 1;
}



u64 syscall_uid_get(void){
	return THREAD_DATA->process->uid;
}



u64 syscall_uid_set(u64 uid){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->uid=uid;
		return 1;
	}
	return 0;
}



u64 syscall_uid_get_name(u64 uid,char* buffer,u32 buffer_length){
	if (buffer_length>syscall_get_user_pointer_max_length((u64)buffer)){
		return 0;
	}
	return uid_get_name(uid,buffer,buffer_length);
}
