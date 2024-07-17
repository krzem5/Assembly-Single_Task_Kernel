#include <kernel/drive/drive.h>
#include <kernel/error/error.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "fs"



static omm_allocator_t* KERNEL_INIT_WRITE _fs_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fs_descriptor_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE fs_handle_type=0;
KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE fs_descriptor_handle_type=0;



static void _delete_nodes_recursive(vfs_node_t* node){
	while (node->relatives.child){
		_delete_nodes_recursive(node->relatives.child);
	}
	vfs_node_dettach_child(node);
	vfs_node_delete(node);
}



static void _fs_handle_destructor(handle_t* handle){
	filesystem_t* fs=KERNEL_CONTAINEROF(handle,filesystem_t,handle);
	if (fs->descriptor->config->deinit_callback){
		fs->descriptor->config->deinit_callback(fs);
	}
	if (fs->root){
		_delete_nodes_recursive(fs->root);
	}
	handle_release(&(fs->descriptor->handle));
	omm_dealloc(_fs_allocator,fs);
}



KERNEL_EARLY_EARLY_INIT(){
	_fs_allocator=omm_init("kernel.fs",sizeof(filesystem_t),8,4);
	rwlock_init(&(_fs_allocator->lock));
	fs_handle_type=handle_alloc("kernel.fs",0,_fs_handle_destructor);
}



KERNEL_PUBLIC void fs_register_descriptor(const filesystem_descriptor_config_t* config,filesystem_descriptor_t** out){
	LOG("Registering filesystem descriptor '%s'...",config->name);
	if (!fs_descriptor_handle_type){
		fs_descriptor_handle_type=handle_alloc("kernel.fs.descriptor",0,NULL);
	}
	if (!_fs_descriptor_allocator){
		_fs_descriptor_allocator=omm_init("kernel.fs.descriptor",sizeof(filesystem_descriptor_t),8,2);
		rwlock_init(&(_fs_descriptor_allocator->lock));
	}
	filesystem_descriptor_t* descriptor=omm_alloc(_fs_descriptor_allocator);
	descriptor->config=config;
	handle_new(fs_descriptor_handle_type,&(descriptor->handle));
	*out=descriptor;
	if (!config->load_callback){
		return;
	}
	HANDLE_FOREACH(partition_handle_type){
		partition_t* partition=KERNEL_CONTAINEROF(handle,partition_t,handle);
		if (partition->fs){
			continue;
		}
		config->load_callback(partition);
	}
}



KERNEL_PUBLIC void fs_unregister_descriptor(filesystem_descriptor_t* descriptor){
	LOG("Unregistering filesystem descriptor '%s'...",descriptor->config->name);
	handle_destroy(&(descriptor->handle));
	omm_dealloc(_fs_descriptor_allocator,descriptor);
}



KERNEL_PUBLIC filesystem_t* fs_create(filesystem_descriptor_t* descriptor){
	handle_acquire(&(descriptor->handle));
	filesystem_t* out=omm_alloc(_fs_allocator);
	handle_new(fs_handle_type,&(out->handle));
	out->descriptor=descriptor;
	out->functions=NULL;
	out->partition=NULL;
	out->extra_data=NULL;
	out->root=NULL;
	mem_fill(out->uuid,16,0);
	out->is_mounted=0;
	return out;
}



KERNEL_PUBLIC filesystem_t* fs_load(partition_t* partition){
	HANDLE_FOREACH(fs_descriptor_handle_type){
		const filesystem_descriptor_t* descriptor=KERNEL_CONTAINEROF(handle,const filesystem_descriptor_t,handle);
		if (!descriptor->config->load_callback){
			continue;
		}
		filesystem_t* out=descriptor->config->load_callback(partition);
		if (out){
			handle_release(handle);
			return out;
		}
	}
	return NULL;
}



KERNEL_PUBLIC bool fs_format(partition_t* partition,const filesystem_descriptor_t* descriptor){
	return (descriptor->config->format_callback?descriptor->config->format_callback(partition):0);
}



error_t syscall_fs_get_next(handle_id_t fs_handle_id){
	handle_descriptor_t* fs_handle_descriptor=handle_get_descriptor(fs_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(fs_handle_descriptor->tree),(fs_handle_id?fs_handle_id+1:0));
	return (rb_node?rb_node->key:0);
}



error_t syscall_fs_get_data(u64 fs_handle_id,KERNEL_USER_POINTER filesystem_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(filesystem_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* fs_handle=handle_lookup_and_acquire(fs_handle_id,fs_handle_type);
	if (!fs_handle){
		return ERROR_INVALID_HANDLE;
	}
	filesystem_t* fs=KERNEL_CONTAINEROF(fs_handle,filesystem_t,handle);
	str_copy(fs->descriptor->config->name,(char*)(buffer->type),sizeof(buffer->type));
	if (fs->partition){
		buffer->partition=fs->partition->handle.rb_node.key;
		mem_copy((void*)(buffer->guid),fs->partition->guid,sizeof(buffer->guid));
	}
	else{
		buffer->partition=0;
		mem_fill((void*)(buffer->guid),sizeof(buffer->guid),0);
	}
	mem_copy((void*)(buffer->uuid),fs->uuid,sizeof(buffer->uuid));
	if (!fs->is_mounted||!vfs_path(fs->root,(char*)(buffer->mount_path),sizeof(buffer->mount_path))){
		buffer->mount_path[0]=0;
	}
	handle_release(fs_handle);
	return ERROR_OK;
}



KERNEL_AWAITS error_t syscall_fs_mount(handle_id_t fs_handle_id,KERNEL_USER_POINTER const char* path){
	u64 path_length=syscall_get_string_length((const char*)path);
	if (!path_length||path_length>4095){
		return ERROR_INVALID_ARGUMENT(1);
	}
	char buffer[4096];
	mem_copy(buffer,(const char*)path,path_length);
	buffer[path_length]=0;
	handle_t* fs_handle=handle_lookup_and_acquire(fs_handle_id,fs_handle_type);
	if (!fs_handle){
		return ERROR_INVALID_HANDLE;
	}
	error_t out=vfs_mount(KERNEL_CONTAINEROF(fs_handle,filesystem_t,handle),buffer,1);
	handle_release(fs_handle);
	return out;
}



error_t syscall_fs_format(handle_id_t partition_handle_id,handle_id_t fs_descriptor_handle_id){
	ERROR("syscall_fs_format(%p,%p)",partition_handle_id,fs_descriptor_handle_id);
	return 0;
}



error_t syscall_fs_descriptor_get_next(handle_id_t fs_descriptor_handle_id){
	handle_descriptor_t* fs_handle_descriptor=handle_get_descriptor(fs_descriptor_handle_type);
	rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(fs_handle_descriptor->tree),(fs_descriptor_handle_id?fs_descriptor_handle_id+1:0));
	return (rb_node?rb_node->key:0);
}



error_t syscall_fs_descriptor_get_data(u64 fs_descriptor_handle_id,KERNEL_USER_POINTER filesystem_descriptor_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(filesystem_descriptor_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* fs_descriptor_handle=handle_lookup_and_acquire(fs_descriptor_handle_id,fs_descriptor_handle_type);
	if (!fs_descriptor_handle){
		return ERROR_INVALID_HANDLE;
	}
	const filesystem_descriptor_t* fs_descriptor=KERNEL_CONTAINEROF(fs_descriptor_handle,const filesystem_descriptor_t,handle);
	str_copy(fs_descriptor->config->name,(char*)(buffer->name),sizeof(buffer->name));
	buffer->flags=0;
	if (fs_descriptor->config->format_callback){
		buffer->flags|=FS_DESCRIPTOR_USER_FLAG_CAN_FORMAT;
	}
	handle_release(fs_descriptor_handle);
	return ERROR_OK;
}
