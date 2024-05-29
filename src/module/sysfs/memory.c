#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/module/module.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_memory"



static vfs_node_t* KERNEL_INIT_WRITE _sysfs_memory_root;
static vfs_node_t* KERNEL_INIT_WRITE _sysfs_memory_pmm_counter_root;
static vfs_node_t* KERNEL_INIT_WRITE _sysfs_memory_object_counter_root;



static void _init_memory_load_balancer_data(void){
	vfs_node_t* root=dynamicfs_create_node(_sysfs_memory_root,"lb",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(root);
	dynamicfs_set_root_only(dynamicfs_create_node(root,"hit",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->hit_count))));
	dynamicfs_set_root_only(dynamicfs_create_node(root,"miss",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->miss_count))));
	dynamicfs_set_root_only(dynamicfs_create_node(root,"late_miss",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->miss_locked_count))));
}



static void _pmm_counter_listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,pmm_counter_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const pmm_counter_descriptor_t* descriptor=KERNEL_CONTAINEROF(handle,const pmm_counter_descriptor_t,handle);
		vfs_node_t* node=dynamicfs_create_node(_sysfs_memory_pmm_counter_root,descriptor->name,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_set_root_only(node);
		dynamicfs_set_root_only(dynamicfs_create_node(node,"count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(descriptor->count))));
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
	handle_release(handle);
}



static void _omm_allocator_listener(u64 object,u32 type){
	handle_t* handle=handle_lookup_and_acquire(object,omm_handle_type);
	if (!handle){
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const omm_allocator_t* allocator=KERNEL_CONTAINEROF(handle,const omm_allocator_t,handle);
		vfs_node_t* node=dynamicfs_create_node(_sysfs_memory_object_counter_root,allocator->name,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_set_root_only(node);
		dynamicfs_set_root_only(dynamicfs_create_node(node,"alloc_count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(allocator->allocation_count))));
		dynamicfs_set_root_only(dynamicfs_create_node(node,"dealloc_count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(allocator->deallocation_count))));
	}
	else if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
	handle_release(handle);
}



static void _pmm_counter_update_notification_thread(void){
	notification2_consumer_t* consumer=notification2_consumer_create(&(handle_get_descriptor(pmm_counter_handle_type)->notification_dispatcher));
	HANDLE_FOREACH(pmm_counter_handle_type){
		_pmm_counter_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	}
	while (1){
		notification2_t notification;
		if (!notification2_consumer_get(consumer,1,&notification)){
			continue;
		}
		handle_t* handle=handle_lookup_and_acquire(notification.object,pmm_counter_handle_type);
		if (!handle){
			continue;
		}
		_pmm_counter_listener(handle->rb_node.key,notification.type);
		handle_release(handle);
	}
}



static void _omm_allocator_update_notification_thread(void){
	notification2_consumer_t* consumer=notification2_consumer_create(&(handle_get_descriptor(omm_handle_type)->notification_dispatcher));
	HANDLE_FOREACH(omm_handle_type){
		_omm_allocator_listener(handle->rb_node.key,NOTIFICATION_TYPE_HANDLE_CREATE);
	}
	while (1){
		notification2_t notification;
		if (!notification2_consumer_get(consumer,1,&notification)){
			continue;
		}
		handle_t* handle=handle_lookup_and_acquire(notification.object,omm_handle_type);
		if (!handle){
			continue;
		}
		_omm_allocator_listener(handle->rb_node.key,notification.type);
		handle_release(handle);
	}
}



MODULE_POSTINIT(){
	LOG("Creating memory subsystem...");
	_sysfs_memory_root=dynamicfs_create_node(sysfs->root,"mem",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	_init_memory_load_balancer_data();
	_sysfs_memory_pmm_counter_root=dynamicfs_create_node(_sysfs_memory_root,"physical",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(_sysfs_memory_pmm_counter_root);
	_sysfs_memory_object_counter_root=dynamicfs_create_node(_sysfs_memory_root,"object",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(_sysfs_memory_object_counter_root);
	thread_create_kernel_thread(NULL,"sysfs.memory.pmm.update.notification",_pmm_counter_update_notification_thread,0);
	thread_create_kernel_thread(NULL,"sysfs.memory.omm.update.notification",_omm_allocator_update_notification_thread,0);
}
