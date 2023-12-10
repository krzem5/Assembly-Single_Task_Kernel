#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "fs"



static omm_allocator_t* _fs_allocator=NULL;
static omm_allocator_t* _fs_descriptor_allocator=NULL;

KERNEL_PUBLIC handle_type_t fs_handle_type=0;
KERNEL_PUBLIC handle_type_t fs_descriptor_handle_type=0;



static void _fs_handle_destructor(handle_t* handle){
	filesystem_t* fs=handle->object;
	WARN("Delete filesystem: %p",fs);
	handle_release(&(fs->descriptor->handle));
	omm_dealloc(_fs_allocator,fs);
}



KERNEL_PUBLIC filesystem_descriptor_t* fs_register_descriptor(const filesystem_descriptor_config_t* config){
	LOG("Registering filesystem descriptor '%s'...",config->name);
	if (!fs_descriptor_handle_type){
		fs_descriptor_handle_type=handle_alloc("fs_descriptor",NULL);
	}
	if (!_fs_descriptor_allocator){
		_fs_descriptor_allocator=omm_init("fs_descriptor",sizeof(filesystem_descriptor_t),8,2,pmm_alloc_counter("omm_fs_descriptor"));
		spinlock_init(&(_fs_descriptor_allocator->lock));
	}
	filesystem_descriptor_t* out=omm_alloc(_fs_descriptor_allocator);
	out->config=config;
	handle_new(out,fs_descriptor_handle_type,&(out->handle));
	handle_finish_setup(&(out->handle));
	if (!config->load_callback){
		return out;
	}
	HANDLE_FOREACH(partition_handle_type){
		partition_t* partition=handle->object;
		if (partition->fs){
			continue;
		}
		config->load_callback(partition);
	}
	return out;
}



KERNEL_PUBLIC void fs_unregister_descriptor(filesystem_descriptor_t* descriptor){
	LOG("Unregistering filesystem descriptor '%s'...",descriptor->config->name);
	handle_destroy(&(descriptor->handle));
	omm_dealloc(_fs_descriptor_allocator,descriptor);
}



KERNEL_PUBLIC filesystem_t* fs_create(filesystem_descriptor_t* descriptor){
	handle_acquire(&(descriptor->handle));
	if (!_fs_allocator){
		_fs_allocator=omm_init("fs",sizeof(filesystem_t),8,4,pmm_alloc_counter("omm_fs"));
		spinlock_init(&(_fs_allocator->lock));
	}
	if (!fs_handle_type){
		fs_handle_type=handle_alloc("fs",_fs_handle_destructor);
	}
	filesystem_t* out=omm_alloc(_fs_allocator);
	handle_new(out,fs_handle_type,&(out->handle));
	out->descriptor=descriptor;
	out->functions=NULL;
	out->partition=NULL;
	out->extra_data=NULL;
	out->root=NULL;
	memset(out->uuid,0,16);
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC filesystem_t* fs_load(partition_t* partition){
	HANDLE_FOREACH(fs_descriptor_handle_type){
		filesystem_descriptor_t* descriptor=handle->object;
		if (!descriptor->config->load_callback){
			continue;
		}
		filesystem_t* out=descriptor->config->load_callback(partition);
		if (out){
			return out;
		}
	}
	return NULL;
}
