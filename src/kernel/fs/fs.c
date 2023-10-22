#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "fs"



static pmm_counter_descriptor_t _fs_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_fs");
static omm_allocator_t _fs_allocator=OMM_ALLOCATOR_INIT_STRUCT("fs",sizeof(filesystem_t),8,4,&_fs_omm_pmm_counter);



HANDLE_DECLARE_TYPE(FS,{
	filesystem_t* fs=handle->object;
	WARN("Delete filesystem: %p",fs);
	handle_release(&(fs->descriptor->handle));
	omm_dealloc(&_fs_allocator,fs);
});
HANDLE_DECLARE_TYPE(FS_DESCRIPTOR,{});



void fs_register_descriptor(filesystem_descriptor_t* descriptor){
	LOG("Registering filesystem descriptor '%s'...",descriptor->name);
	handle_new(descriptor,HANDLE_TYPE_FS_DESCRIPTOR,&(descriptor->handle));
	HANDLE_FOREACH(HANDLE_TYPE_PARTITION){
		partition_t* partition=handle->object;
		if (partition->fs){
			continue;
		}
		descriptor->load_callback(partition);
	}
}



void fs_unregister_descriptor(filesystem_descriptor_t* descriptor){
	LOG("Unregistering filesystem descriptor '%s'...",descriptor->name);
	handle_destroy(&(descriptor->handle));
}



filesystem_t* fs_create(filesystem_descriptor_t* descriptor){
	handle_acquire(&(descriptor->handle));
	filesystem_t* out=omm_alloc(&_fs_allocator);
	handle_new(out,HANDLE_TYPE_FS,&(out->handle));
	lock_init(&(out->lock));
	out->descriptor=descriptor;
	out->functions=NULL;
	out->root=NULL;
	return out;
}



filesystem_t* fs_load(partition_t* partition){
	HANDLE_FOREACH(HANDLE_TYPE_FS_DESCRIPTOR){
		filesystem_descriptor_t* descriptor=handle->object;
		filesystem_t* out=descriptor->load_callback(partition);
		if (out){
			return out;
		}
	}
	return NULL;
}
