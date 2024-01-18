#include <fuse/fuse.h>
#include <fuse/fuse_registers.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/fs.h>
#include <virtio/fs_registers.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio_fs"



static omm_allocator_t* _virtio_fs_device_allocator=NULL;



static _Bool _virtio_driver_init(virtio_device_t* device,u64 features){
	virtio_fs_config_t config;
	for (u32 i=0;i<sizeof(config.raw_data)/sizeof(u32);i++){
		config.raw_data[i]=virtio_read(device->device_field+i*sizeof(u32),4);
	}
	if (!config.tag[0]||!config.num_request_queues){
		return 0;
	}
	virtio_queue_t* hiprioq=virtio_init_queue(device,0);
	if (!hiprioq){
		return 0;
	}
	virtio_queue_t* loprioq=virtio_init_queue(device,1);
	if (!loprioq){
		return 0;
	}
	virtio_fs_device_t* fs_device=omm_alloc(_virtio_fs_device_allocator);
	fs_device->hiprioq=hiprioq;
	fs_device->loprioq=loprioq;
	config.tag[35]=0;
	INFO("Creating File System device...");
	INFO("Filesystem name: %s",config.tag);
	virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
	virtio_fs_fuse_init(fs_device);
	fuse_create_filesystem(fs_device);
	return 1;
}



static const virtio_device_driver_t _virtio_fs_device_driver={
	"File System Device",
	0x001a,
	0,
	0,
	_virtio_driver_init
};



void virtio_fs_init(void){
	LOG("Initializing VirtIO FS driver...");
	_virtio_fs_device_allocator=omm_init("virtio_fs_device",sizeof(virtio_fs_device_t),8,1,pmm_alloc_counter("omm_virtio_fs_device"));
	spinlock_init(&(_virtio_fs_device_allocator->lock));
	if (!virtio_register_device_driver(&_virtio_fs_device_driver)){
		ERROR("Unable to register VirtIO FS driver");
	}
}



void virtio_fs_fuse_init(virtio_fs_device_t* fs_device){
	fuse_init_in_t* fuse_init_in=amm_alloc(sizeof(fuse_init_in_t));
	fuse_init_in->header.len=sizeof(fuse_init_in_t);
	fuse_init_in->header.opcode=FUSE_OPCODE_INIT;
	fuse_init_in->header.total_extlen=0;
	fuse_init_in->major=FUSE_VERSION_MAJOR;
	fuse_init_in->minor=FUSE_VERSION_MINOR;
	fuse_init_in->max_readahead=0;
	fuse_init_in->flags=0;
	fuse_init_in->flags2=0;
	fuse_init_out_t* fuse_init_out=amm_alloc(sizeof(fuse_init_out_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)fuse_init_in),
			sizeof(fuse_init_in_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)fuse_init_out),
			sizeof(fuse_init_out_t)
		}
	};
	virtio_queue_transfer(fs_device->hiprioq,buffers,1,1);
	virtio_queue_wait(fs_device->hiprioq);
	virtio_queue_pop(fs_device->hiprioq,NULL);
	amm_dealloc(fuse_init_in);
	if (fuse_init_out->header.len!=sizeof(fuse_init_out_t)||fuse_init_out->major!=FUSE_VERSION_MAJOR||fuse_init_out->minor!=FUSE_VERSION_MINOR){
		WARN("Invalid FUSE initialization response");
	}
	amm_dealloc(fuse_init_out);
}



fuse_getattr_out_t* virtio_fs_fuse_getattr(virtio_fs_device_t* fs_device,fuse_node_id_t fuse_node_id){
	fuse_getattr_in_t* fuse_getattr_in=amm_alloc(sizeof(fuse_getattr_in_t));
	fuse_getattr_in->header.len=sizeof(fuse_getattr_in_t);
	fuse_getattr_in->header.opcode=FUSE_OPCODE_GETATTR;
	fuse_getattr_in->header.nodeid=fuse_node_id;
	fuse_getattr_in->header.total_extlen=0;
	fuse_getattr_in->getattr_flags=0;
	fuse_getattr_out_t* fuse_getattr_out=amm_alloc(sizeof(fuse_getattr_out_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)fuse_getattr_in),
			sizeof(fuse_getattr_in_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)fuse_getattr_out),
			sizeof(fuse_getattr_out_t)
		}
	};
	virtio_queue_transfer(fs_device->loprioq,buffers,1,1);
	virtio_queue_wait(fs_device->loprioq);
	virtio_queue_pop(fs_device->loprioq,NULL);
	amm_dealloc(fuse_getattr_in);
	return fuse_getattr_out;
}
