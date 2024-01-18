#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/resource/resource.h>
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
	config.tag[35]=0;
	INFO("Creating File System device...");
	INFO("Filesystem name: %s",config.tag);
	virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
	fuse_init_in_t* fuse_init_in=amm_alloc(sizeof(fuse_init_in_t));
	fuse_init_in->header.len=sizeof(fuse_init_in_t);
	fuse_init_in->header.opcode=FUSE_INIT;
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
	virtio_queue_transfer(hiprioq,buffers,1,1);
	virtio_queue_wait(hiprioq);
	virtio_queue_pop(hiprioq,NULL);
	amm_dealloc(fuse_init_in);
	if (fuse_init_out->header.len!=sizeof(fuse_init_out_t)){
		WARN("Invalid FUSE initialization responce");
		return 0;
	}
	WARN("major=%u",fuse_init_out->major);
	WARN("minor=%u",fuse_init_out->minor);
	WARN("max_readahead=%u",fuse_init_out->max_readahead);
	WARN("flags=%u",fuse_init_out->flags);
	WARN("max_background=%u",fuse_init_out->max_background);
	WARN("congestion_threshold=%u",fuse_init_out->congestion_threshold);
	WARN("max_write=%u",fuse_init_out->max_write);
	WARN("time_gran=%u",fuse_init_out->time_gran);
	WARN("max_pages=%u",fuse_init_out->max_pages);
	amm_dealloc(fuse_init_out);
	// panic("AAA");
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



// virtio_fs_resp_display_info_t* virtio_fs_command_get_display_info(virtio_fs_device_t* fs_device){
// 	virtio_fs_control_header_t* request=amm_alloc(sizeof(virtio_fs_control_header_t));
// 	request->type=VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
// 	request->flags=VIRTIO_GPU_FLAG_FENCE;
// 	request->fence_id=0;
// 	virtio_fs_resp_display_info_t* response=amm_alloc(sizeof(virtio_fs_resp_display_info_t));
// 	virtio_buffer_t buffers[2]={
// 		{
// 			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
// 			sizeof(virtio_fs_control_header_t)
// 		},
// 		{
// 			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
// 			sizeof(virtio_fs_resp_display_info_t)
// 		}
// 	};
// 	virtio_queue_transfer(fs_device->controlq,buffers,1,1);
// 	virtio_queue_wait(fs_device->controlq);
// 	virtio_queue_pop(fs_device->controlq,NULL);
// 	amm_dealloc(request);
// 	if (response->header.type==VIRTIO_GPU_RESP_OK_DISPLAY_INFO){
// 		return response;
// 	}
// 	ERROR("virtio_fs_command_get_display_info failed");
// 	amm_dealloc(response);
// 	return NULL;
// }
