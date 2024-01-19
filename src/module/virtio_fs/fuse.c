#include <fuse/fuse.h>
#include <fuse/fuse_registers.h>
#include <kernel/clock/clock.h>
#include <kernel/fs/fs.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#include <virtio/fs.h>
#define KERNEL_LOG_NAME "fuse"



typedef struct _FUSE_VFS_NODE{
	vfs_node_t header;
	fuse_node_id_t node_id;
	fuse_file_handle_t file_handle;
	u64 data_valid_end_time;
	u64 size;
} fuse_vfs_node_t;



static omm_allocator_t* _fuse_vfs_node_allocator=NULL;
static filesystem_descriptor_t* _fuse_filesystem_descriptor=NULL;



static vfs_node_t* _open_node(filesystem_t* fs,fuse_node_id_t node_id,const string_t* name){
	virtio_fs_device_t* fs_device=fs->extra_data;
	u64 file_handle=virtio_fs_fuse_open(fs_device,node_id);
	fuse_getattr_out_t* fuse_getattr_out=virtio_fs_fuse_getattr(fs_device,node_id,file_handle);
	fuse_vfs_node_t* out=(fuse_vfs_node_t*)vfs_node_create(fs,name);
	out->header.flags|=(fuse_getattr_out->attr.mode&0777)<<VFS_NODE_PERMISSION_SHIFT;
	out->header.time_access=fuse_getattr_out->attr.atime*1000000000ull+fuse_getattr_out->attr.atimensec;
	out->header.time_modify=fuse_getattr_out->attr.mtime*1000000000ull+fuse_getattr_out->attr.mtimensec;
	out->header.time_change=fuse_getattr_out->attr.ctime*1000000000ull+fuse_getattr_out->attr.ctimensec;
	out->header.time_birth=0;
	out->header.gid=fuse_getattr_out->attr.gid;
	out->header.uid=fuse_getattr_out->attr.uid;
	out->node_id=node_id;
	out->file_handle=file_handle;
	out->data_valid_end_time=clock_get_time()+fuse_getattr_out->attr_valid*1000000000ull+fuse_getattr_out->attr_valid_nsec;
	out->size=fuse_getattr_out->attr.size;
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
	out->file_handle=0;
	return (vfs_node_t*)out;
}



static void _fuse_delete(vfs_node_t* node){
	omm_dealloc(_fuse_vfs_node_allocator,node);
}



static vfs_node_t* _fuse_lookup(vfs_node_t* node,const string_t* name){
	fuse_vfs_node_t* fuse_node=(fuse_vfs_node_t*)node;
	fuse_lookup_out_t* fuse_lookup_out=virtio_fs_fuse_lookup(node->fs->extra_data,fuse_node->node_id,name->data,name->length+1);
	if (fuse_lookup_out->header.error){
		amm_dealloc(fuse_lookup_out);
		return NULL;
	}
	// reuse fuse_lookup_out->attr
	// cache name based on fuse_lookup_out->entry_valid
	vfs_node_t* out=_open_node(node->fs,fuse_lookup_out->nodeid,name);
	amm_dealloc(fuse_lookup_out);
	return out;
}


static u64 _fuse_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	fuse_vfs_node_t* fuse_node=(fuse_vfs_node_t*)node;
	fuse_read_out_t* buffer=amm_alloc(sizeof(fuse_read_out_t)+sizeof(fuse_dirent_t)+256);
_retry_read:
	virtio_fs_fuse_read(node->fs->extra_data,fuse_node->node_id,fuse_node->file_handle,pointer,buffer,sizeof(fuse_read_out_t)+sizeof(fuse_dirent_t)+256,1);
	if (buffer->header.error){
		amm_dealloc(buffer);
		return 0;
	}
	fuse_dirent_t* dirent=(fuse_dirent_t*)(buffer->data);
	pointer=(dirent->namelen?dirent->off:0);
	if (buffer->header.len){
		if (dirent->namelen==1&&dirent->name[0]=='.'){
			goto _retry_read;
		}
		if (dirent->namelen==2&&dirent->name[0]=='.'&&dirent->name[1]=='.'){
			goto _retry_read;
		}
		*out=smm_alloc(dirent->name,dirent->namelen);
	}
	amm_dealloc(buffer);
	return pointer;
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



void fuse_init(void){
	LOG("Initializing FUSE driver...");
	_fuse_vfs_node_allocator=omm_init("fuse_vfs_node",sizeof(fuse_vfs_node_t),8,4,pmm_alloc_counter("omm_fuse_vfs_node"));
	spinlock_init(&(_fuse_vfs_node_allocator->lock));
	_fuse_filesystem_descriptor=fs_register_descriptor(&_fuse_filesystem_descriptor_config);
}



filesystem_t* fuse_create_filesystem(virtio_fs_device_t* fs_device){
	filesystem_t* out=fs_create(_fuse_filesystem_descriptor);
	out->functions=&_fuse_functions;
	out->extra_data=fs_device;
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=_open_node(out,FUSE_ROOT_ID,root_name);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT;
	return out;
}
