#include <kernel/error/error.h>
#include <kernel/id/flags.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
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
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "uid"



typedef struct _UID_GROUP{
	rb_tree_node_t rb_node;
} uid_group_t;



typedef struct _UID_DATA{
	rb_tree_node_t rb_node;
	string_t* name;
	rb_tree_t group_tree;
	id_flags_t flags;
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
	if (uid_create(0,"root")!=ERROR_OK||uid_add_group(0,0)!=ERROR_OK){
		panic("Unable to create root user");
	}
}



KERNEL_PUBLIC error_t uid_create(uid_t uid,const char* name){
	spinlock_acquire_exclusive(&_uid_global_lock);
	if (rb_tree_lookup_node(&_uid_tree,uid)){
		spinlock_release_exclusive(&_uid_global_lock);
		return ERROR_ALREADY_PRESENT;
	}
	uid_data_t* uid_data=omm_alloc(_uid_data_allocator);
	uid_data->rb_node.key=uid;
	uid_data->name=smm_alloc(name,0);
	uid_data->flags=0;
	rb_tree_init(&(uid_data->group_tree));
	rb_tree_insert_node(&_uid_tree,&(uid_data->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	return ERROR_OK;
}



KERNEL_PUBLIC error_t uid_delete(uid_t uid){
	spinlock_acquire_exclusive(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data){
		spinlock_release_exclusive(&_uid_global_lock);
		return ERROR_NOT_FOUND;
	}
	smm_dealloc(uid_data->name);
	while (1){
		uid_group_t* uid_group=(uid_group_t*)rb_tree_lookup_increasing_node(&(uid_data->group_tree),0);
		if (!uid_group){
			break;
		}
		rb_tree_remove_node(&(uid_data->group_tree),&(uid_group->rb_node));
		omm_dealloc(_uid_group_allocator,uid_group);
	}
	rb_tree_remove_node(&_uid_tree,&(uid_data->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	omm_dealloc(_uid_data_allocator,uid_data);
	return ERROR_OK;
}



KERNEL_PUBLIC error_t uid_add_group(uid_t uid,gid_t gid){
	spinlock_acquire_exclusive(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data||rb_tree_lookup_node(&(uid_data->group_tree),gid)){
		spinlock_release_exclusive(&_uid_global_lock);
		return (uid_data?ERROR_ALREADY_PRESENT:ERROR_NOT_FOUND);
	}
	uid_group_t* uid_group=omm_alloc(_uid_group_allocator);
	uid_group->rb_node.key=gid;
	rb_tree_insert_node(&(uid_data->group_tree),&(uid_group->rb_node));
	spinlock_release_exclusive(&_uid_global_lock);
	return ERROR_OK;
}



KERNEL_PUBLIC error_t uid_has_group(uid_t uid,gid_t gid){
	spinlock_acquire_shared(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	error_t out=(uid_data?!!rb_tree_lookup_node(&(uid_data->group_tree),gid):ERROR_NOT_FOUND);
	spinlock_release_shared(&_uid_global_lock);
	return out;
}



KERNEL_PUBLIC error_t uid_remove_group(uid_t uid,gid_t gid){
	spinlock_acquire_exclusive(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data){
		spinlock_release_exclusive(&_uid_global_lock);
		return ERROR_NOT_FOUND;
	}
	uid_group_t* uid_group=(uid_group_t*)rb_tree_lookup_node(&(uid_data->group_tree),gid);
	if (!uid_group){
		spinlock_release_exclusive(&_uid_global_lock);
		return ERROR_NOT_FOUND;
	}
	rb_tree_remove_node(&(uid_data->group_tree),&(uid_group->rb_node));
	omm_dealloc(_uid_group_allocator,uid_group);
	spinlock_release_exclusive(&_uid_global_lock);
	return ERROR_OK;
}



KERNEL_PUBLIC error_t uid_get_name(uid_t uid,char* buffer,u32 buffer_length){
	if (!buffer_length){
		return ERROR_NO_SPACE;
	}
	spinlock_acquire_shared(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data){
		spinlock_release_shared(&_uid_global_lock);
		return ERROR_NOT_FOUND;
	}
	strcpy(buffer,uid_data->name->data,buffer_length);
	spinlock_release_shared(&_uid_global_lock);
	return ERROR_OK;
}



KERNEL_PUBLIC id_flags_t uid_get_flags(uid_t uid){
	spinlock_acquire_shared(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data){
		spinlock_release_shared(&_uid_global_lock);
		return ERROR_NOT_FOUND;
	}
	id_flags_t out=uid_data->flags;
	spinlock_release_shared(&_uid_global_lock);
	return out;
}



KERNEL_PUBLIC error_t uid_set_flags(uid_t uid,id_flags_t clear,id_flags_t set){
	spinlock_acquire_shared(&_uid_global_lock);
	uid_data_t* uid_data=(uid_data_t*)rb_tree_lookup_node(&_uid_tree,uid);
	if (!uid_data){
		spinlock_release_shared(&_uid_global_lock);
		return ERROR_NOT_FOUND;
	}
	uid_data->flags=(uid_data->flags&(~clear))|set;
	spinlock_release_shared(&_uid_global_lock);
	return ERROR_OK;
}



error_t syscall_uid_get(void){
	return THREAD_DATA->process->uid;
}



error_t syscall_uid_set(u64 uid){
	if (process_is_root()){
		THREAD_DATA->process->uid=uid;
		return ERROR_OK;
	}
	return ERROR_DENIED;
}



error_t syscall_uid_get_name(u64 uid,KERNEL_USER_POINTER char* buffer,u32 buffer_length){
	if (!buffer_length){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (buffer_length>syscall_get_user_pointer_max_length((char*)buffer)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	return uid_get_name(uid,(char*)buffer,buffer_length);
}
