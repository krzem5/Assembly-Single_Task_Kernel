#include <kernel/fs/fs.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#include <virtio/fs.h>
#include <virtio/fs_registers.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio_fs"



static omm_allocator_t* _virtio_fs_device_allocator=NULL;
static omm_allocator_t* _virtiofs_vfs_node_allocator=NULL;
static filesystem_descriptor_t* _virtio_filesystem_descriptor=NULL;



static vfs_node_t* _virtiofs_create(void){
	return omm_alloc(_virtiofs_vfs_node_allocator);
}



static void _virtiofs_delete(vfs_node_t* node){
	omm_dealloc(_virtiofs_vfs_node_allocator,node);
}



static vfs_node_t* _virtiofs_lookup(vfs_node_t* node,const string_t* name){
	panic("_virtiofs_lookup");
}



static u64 _virtiofs_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	panic("_virtiofs_iterate");
}



static u64 _virtiofs_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	panic("_virtiofs_read");
}



static u64 _virtiofs_resize(vfs_node_t* node,s64 size,u32 flags){
	// panic("_virtiofs_resize");
	return 0;
}



static const vfs_functions_t _virtiofs_functions={
	_virtiofs_create,
	_virtiofs_delete,
	_virtiofs_lookup,
	_virtiofs_iterate,
	NULL,
	NULL,
	_virtiofs_read,
	NULL,
	_virtiofs_resize,
	NULL
};



static void _virtiofs_fs_deinit(filesystem_t* fs){
	panic("_virtiofs_fs_deinit");
}



static const filesystem_descriptor_config_t _virtio_fs_filesystem_descriptor_config={
	"virtiofs",
	_virtiofs_fs_deinit,
	NULL
};



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
	if (fuse_init_out->header.len!=sizeof(fuse_init_out_t)){
		WARN("Invalid FUSE initialization responce");
		return 0;
	}
	// WARN("major=%u",fuse_init_out->major);
	// WARN("minor=%u",fuse_init_out->minor);
	// WARN("max_readahead=%u",fuse_init_out->max_readahead);
	// WARN("flags=%u",fuse_init_out->flags);
	// WARN("max_background=%u",fuse_init_out->max_background);
	// WARN("congestion_threshold=%u",fuse_init_out->congestion_threshold);
	// WARN("max_write=%u",fuse_init_out->max_write);
	// WARN("time_gran=%u",fuse_init_out->time_gran);
	// WARN("max_pages=%u",fuse_init_out->max_pages);
	amm_dealloc(fuse_init_out);
	filesystem_t* fs=fs_create(_virtio_filesystem_descriptor);
	fs->functions=&_virtiofs_functions;
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	fs->root=vfs_node_create(fs,root_name);
	fs->root->flags|=VFS_NODE_FLAG_PERMANENT|VFS_NODE_TYPE_DIRECTORY;
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
	_virtiofs_vfs_node_allocator=omm_init("virtiofs_vfs_node",sizeof(vfs_node_t),8,4,pmm_alloc_counter("omm_virtiofs_vfs_node"));
	spinlock_init(&(_virtiofs_vfs_node_allocator->lock));
	_virtio_filesystem_descriptor=fs_register_descriptor(&_virtio_fs_filesystem_descriptor_config);
	if (!virtio_register_device_driver(&_virtio_fs_device_driver)){
		ERROR("Unable to register VirtIO FS driver");
	}
}
