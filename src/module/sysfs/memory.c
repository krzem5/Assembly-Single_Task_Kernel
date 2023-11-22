#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/notification/notification.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_memory"



static vfs_node_t* _sysfs_memory_root;
static vfs_node_t* _sysfs_memory_pmm_counter_root;
static vfs_node_t* _sysfs_memory_object_counter_root;



static void _init_memory_load_balancer_data(void){
	vfs_node_t* root=dynamicfs_create_node(_sysfs_memory_root,"lb",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(root);
	dynamicfs_set_root_only(dynamicfs_create_node(root,"hit",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->hit_count))));
	dynamicfs_set_root_only(dynamicfs_create_node(root,"miss",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->miss_count))));
	dynamicfs_set_root_only(dynamicfs_create_node(root,"late_miss",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(pmm_load_balancer_stats->miss_locked_count))));
}



static void _pmm_counter_listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const pmm_counter_descriptor_t* descriptor=handle->object;
		vfs_node_t* node=dynamicfs_create_node(_sysfs_memory_pmm_counter_root,descriptor->name,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_set_root_only(node);
		dynamicfs_set_root_only(dynamicfs_create_node(node,"count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(descriptor->count))));
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _sysfs_memory_pmm_counter_notification_listener={
	_pmm_counter_listener
};



static void _omm_allocator_listener(void* object,u32 type){
	handle_t* handle=object;
	if (type==NOTIFICATION_TYPE_HANDLE_CREATE){
		const omm_allocator_t* allocator=handle->object;
		vfs_node_t* node=dynamicfs_create_node(_sysfs_memory_object_counter_root,allocator->name,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_set_root_only(node);
		dynamicfs_set_root_only(dynamicfs_create_node(node,"alloc_count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(allocator->allocation_count))));
		dynamicfs_set_root_only(dynamicfs_create_node(node,"dealloc_count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&(allocator->deallocation_count))));
		return;
	}
	if (type==NOTIFICATION_TYPE_HANDLE_DELETE){
		WARN("%p",handle);
	}
}



static notification_listener_t _sysfs_memory_omm_allocator_notification_listener={
	_omm_allocator_listener
};



void sysfs_memory_init(void){
	LOG("Creating memory subsystem...");
	_sysfs_memory_root=dynamicfs_create_node(sysfs->root,"mem",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	_init_memory_load_balancer_data();
	_sysfs_memory_pmm_counter_root=dynamicfs_create_node(_sysfs_memory_root,"physical",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(_sysfs_memory_pmm_counter_root);
	handle_register_notification_listener(HANDLE_TYPE_PMM_COUNTER,&_sysfs_memory_pmm_counter_notification_listener);
	_sysfs_memory_object_counter_root=dynamicfs_create_node(_sysfs_memory_root,"object",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	dynamicfs_set_root_only(_sysfs_memory_object_counter_root);
	handle_register_notification_listener(omm_handle_type,&_sysfs_memory_omm_allocator_notification_listener);
}
