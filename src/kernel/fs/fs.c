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



PMM_DECLARE_COUNTER(OMM_FS);



static omm_allocator_t _fs_allocator=OMM_ALLOCATOR_INIT_STRUCT("fs",sizeof(filesystem_t),8,4,PMM_COUNTER_OMM_FS);



HANDLE_DECLARE_TYPE(FS,{
	filesystem_t* fs=handle->object;
	WARN("Delete fs: %p",fs);
	omm_dealloc(&_fs_allocator,fs);
});



void fs_init(void){
	filesystem_type_t fs_type_index=0;
	for (const filesystem_descriptor_t*const* descriptor=(void*)kernel_section_filesystem_start();(u64)descriptor<kernel_section_filesystem_end();descriptor++){
		*((*descriptor)->var)=fs_type_index;
		fs_type_index++;
	}
}



filesystem_t* fs_create(filesystem_type_t type){
	filesystem_t* out=omm_alloc(&_fs_allocator);
	handle_new(out,HANDLE_TYPE_FS,&(out->handle));
	lock_init(&(out->lock));
	out->type=type;
	out->functions=NULL;
	out->root=NULL;
	return out;
}



filesystem_t* fs_load(partition_t* partition){
	for (const filesystem_descriptor_t*const* descriptor=(void*)kernel_section_filesystem_start();(u64)descriptor<kernel_section_filesystem_end();descriptor++){
		filesystem_t* out=(*descriptor)->load_callback(partition);
		if (out){
			return out;
		}
	}
	return NULL;
}
