#include <fuse/fuse_registers.h>
#include <kernel/clock/clock.h>
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



typedef struct _FUSE_VFS_NODE{
	vfs_node_t header;
	u64 data_valid_end_time;
	u64 size;
	u64 fuse_node_id;
} fuse_vfs_node_t;



static omm_allocator_t* _virtio_fs_device_allocator=NULL;
static omm_allocator_t* _fuse_vfs_node_allocator=NULL;
static filesystem_descriptor_t* _virtio_filesystem_descriptor=NULL;



static vfs_node_t* _open_node(filesystem_t* fs,fuse_node_id_t node_id,const string_t* name){
	virtio_fs_device_t* fs_device=fs->extra_data;
	fuse_getattr_in_t* fuse_getattr_in=amm_alloc(sizeof(fuse_getattr_in_t));
	fuse_getattr_in->header.len=sizeof(fuse_getattr_in_t);
	fuse_getattr_in->header.opcode=FUSE_OPCODE_GETATTR;
	fuse_getattr_in->header.nodeid=node_id;
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
	fuse_vfs_node_t* out=(fuse_vfs_node_t*)vfs_node_create(fs,name);
	out->header.flags|=(fuse_getattr_out->attr.mode&0777)<<VFS_NODE_PERMISSION_SHIFT;
	out->header.time_access=fuse_getattr_out->attr.atime*1000000000ull+fuse_getattr_out->attr.atimensec;
	out->header.time_modify=fuse_getattr_out->attr.mtime*1000000000ull+fuse_getattr_out->attr.mtimensec;
	out->header.time_change=fuse_getattr_out->attr.ctime*1000000000ull+fuse_getattr_out->attr.ctimensec;
	out->header.time_birth=0;
	out->header.gid=fuse_getattr_out->attr.gid;
	out->header.uid=fuse_getattr_out->attr.uid;
	out->data_valid_end_time=clock_get_time()+fuse_getattr_out->attr_valid*1000000000ull+fuse_getattr_out->attr_valid_nsec;
	out->size=fuse_getattr_out->attr.size;
	out->fuse_node_id=node_id;
	switch (fuse_getattr_out->attr.mode&0170000){
		case 0040000:
			out->header.flags|=VFS_NODE_TYPE_DIRECTORY;
			break;
		default:
		case 0100000:
			out->header.flags|=VFS_NODE_TYPE_FILE;
			break;
		case 0120000:
			out->header.flags|=VFS_NODE_TYPE_LINK;
			break;
	}
	amm_dealloc(fuse_getattr_out);
	return (vfs_node_t*)out;
}



static vfs_node_t* _fuse_create(void){
	fuse_vfs_node_t* out=omm_alloc(_fuse_vfs_node_allocator);
	out->data_valid_end_time=0;
	out->size=0;
	out->fuse_node_id=0;
	return (vfs_node_t*)out;
}



static void _fuse_delete(vfs_node_t* node){
	omm_dealloc(_fuse_vfs_node_allocator,node);
}



static vfs_node_t* _fuse_lookup(vfs_node_t* node,const string_t* name){
	panic("_fuse_lookup");
}



static u64 _fuse_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	panic("_fuse_iterate");
}



static u64 _fuse_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	panic("_fuse_read");
}



static u64 _fuse_resize(vfs_node_t* node,s64 size,u32 flags){
	if (!(flags&VFS_NODE_FLAG_RESIZE_RELATIVE)||size){
		return -1;
	}
	return ((fuse_vfs_node_t*)node)->size;
}



static const vfs_functions_t _fuse_functions={
	_fuse_create,
	_fuse_delete,
	_fuse_lookup,
	_fuse_iterate,
	NULL,
	NULL,
	_fuse_read,
	NULL,
	_fuse_resize,
	NULL
};



static void _fuse_fs_deinit(filesystem_t* fs){
	panic("_fuse_fs_deinit");
}



static const filesystem_descriptor_config_t _fuse_filesystem_descriptor_config={
	"fuse",
	_fuse_fs_deinit,
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
	if (fuse_init_out->header.len!=sizeof(fuse_init_out_t)||fuse_init_out->major!=FUSE_VERSION_MAJOR||fuse_init_out->minor!=FUSE_VERSION_MINOR){
		WARN("Invalid FUSE initialization responce");
		return 0;
	}
	amm_dealloc(fuse_init_out);
	filesystem_t* fs=fs_create(_virtio_filesystem_descriptor);
	fs->functions=&_fuse_functions;
	fs->extra_data=fs_device;
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	fs->root=_open_node(fs,FUSE_ROOT_ID,root_name);
	fs->root->flags|=VFS_NODE_FLAG_PERMANENT;
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
	_fuse_vfs_node_allocator=omm_init("fuse_vfs_node",sizeof(fuse_vfs_node_t),8,4,pmm_alloc_counter("omm_fuse_vfs_node"));
	spinlock_init(&(_fuse_vfs_node_allocator->lock));
	_virtio_filesystem_descriptor=fs_register_descriptor(&_fuse_filesystem_descriptor_config);
	if (!virtio_register_device_driver(&_virtio_fs_device_driver)){
		ERROR("Unable to register VirtIO FS driver");
	}
}
