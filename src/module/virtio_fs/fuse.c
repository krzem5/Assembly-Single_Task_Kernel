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



#define FUSE_NODE_NAME_VALID_NS 1000000000



typedef struct _FUSE_VFS_NODE{
	vfs_node_t header;
	fuse_node_id_t node_id;
	fuse_file_handle_t file_handle;
	u64 data_valid_end_time;
	u64 name_valid_end_time;
	u64 size;
} fuse_vfs_node_t;



static omm_allocator_t* _fuse_vfs_node_allocator=NULL;
static filesystem_descriptor_t* _fuse_filesystem_descriptor=NULL;



static void _update_attr(fuse_vfs_node_t* fuse_node){
	fuse_getattr_out_t* fuse_getattr_out=virtio_fs_fuse_getattr(fuse_node->header.fs->extra_data,fuse_node->node_id,fuse_node->file_handle);
	fuse_node->header.flags&=~(VFS_NODE_TYPE_MASK|VFS_NODE_PERMISSION_MASK);
	fuse_node->header.flags|=(fuse_getattr_out->attr.mode&0777)<<VFS_NODE_PERMISSION_SHIFT;
	fuse_node->header.time_access=fuse_getattr_out->attr.atime*1000000000ull+fuse_getattr_out->attr.atimensec;
	fuse_node->header.time_modify=fuse_getattr_out->attr.mtime*1000000000ull+fuse_getattr_out->attr.mtimensec;
	fuse_node->header.time_change=fuse_getattr_out->attr.ctime*1000000000ull+fuse_getattr_out->attr.ctimensec;
	fuse_node->header.time_birth=0;
	fuse_node->header.gid=fuse_getattr_out->attr.gid;
	fuse_node->header.uid=fuse_getattr_out->attr.uid;
	fuse_node->data_valid_end_time=clock_get_time()+fuse_getattr_out->attr_valid*1000000000ull+fuse_getattr_out->attr_valid_nsec;
	fuse_node->size=fuse_getattr_out->attr.size;
	switch (fuse_getattr_out->attr.mode&0170000){
		case 0040000:
			fuse_node->header.flags|=VFS_NODE_TYPE_DIRECTORY;
			break;
		default:
		case 0100000:
			fuse_node->header.flags|=VFS_NODE_TYPE_FILE;
			break;
		case 0120000:
			fuse_node->header.flags|=VFS_NODE_TYPE_LINK;
			break;
	}
	amm_dealloc(fuse_getattr_out);
}



static void _check_for_updates(fuse_vfs_node_t* fuse_node){
	if (clock_get_time()>=fuse_node->data_valid_end_time){
		fuse_getattr_out_t* fuse_getattr_out=virtio_fs_fuse_getattr(fuse_node->header.fs->extra_data,fuse_node->node_id,fuse_node->file_handle);
		fuse_node->header.flags&=~(VFS_NODE_TYPE_MASK|VFS_NODE_PERMISSION_MASK);
		fuse_node->header.flags|=(fuse_getattr_out->attr.mode&0777)<<VFS_NODE_PERMISSION_SHIFT;
		fuse_node->header.time_access=fuse_getattr_out->attr.atime*1000000000ull+fuse_getattr_out->attr.atimensec;
		fuse_node->header.time_modify=fuse_getattr_out->attr.mtime*1000000000ull+fuse_getattr_out->attr.mtimensec;
		fuse_node->header.time_change=fuse_getattr_out->attr.ctime*1000000000ull+fuse_getattr_out->attr.ctimensec;
		fuse_node->header.time_birth=0;
		fuse_node->header.gid=fuse_getattr_out->attr.gid;
		fuse_node->header.uid=fuse_getattr_out->attr.uid;
		fuse_node->data_valid_end_time=clock_get_time()+fuse_getattr_out->attr_valid*1000000000ull+fuse_getattr_out->attr_valid_nsec;
		fuse_node->size=fuse_getattr_out->attr.size;
		switch (fuse_getattr_out->attr.mode&0170000){
			case 0040000:
				fuse_node->header.flags|=VFS_NODE_TYPE_DIRECTORY;
				break;
			default:
			case 0100000:
				fuse_node->header.flags|=VFS_NODE_TYPE_FILE;
				break;
			case 0120000:
				fuse_node->header.flags|=VFS_NODE_TYPE_LINK;
				break;
		}
		amm_dealloc(fuse_getattr_out);
	}
	if (clock_get_time()>=fuse_node->name_valid_end_time&&!(fuse_node->header.flags&VFS_NODE_FLAG_PERMANENT)&&fuse_node->header.relatives.parent){
		fuse_vfs_node_t* parent_fuse_node=(fuse_vfs_node_t*)(fuse_node->header.relatives.parent);
		fuse_lookup_out_t* fuse_lookup_out=virtio_fs_fuse_lookup(fuse_node->header.fs->extra_data,parent_fuse_node->node_id,fuse_node->header.name->data,fuse_node->header.name->length+1);
		if (fuse_lookup_out->header.error){
			vfs_node_dettach_child(&(fuse_node->header));
			fuse_node->name_valid_end_time=0xffffffffffffffffull;
		}
		else{
			fuse_node->name_valid_end_time=clock_get_time()+fuse_lookup_out->entry_valid*1000000000ull+fuse_lookup_out->entry_valid_nsec;
		}
		amm_dealloc(fuse_lookup_out);
	}
}



static vfs_node_t* _open_node(filesystem_t* fs,fuse_node_id_t node_id,const string_t* name){
	virtio_fs_device_t* fs_device=fs->extra_data;
	fuse_vfs_node_t* out=(fuse_vfs_node_t*)vfs_node_create(fs,name);
	out->node_id=node_id;
	out->file_handle=virtio_fs_fuse_open(fs_device,node_id);
	_update_attr(out);
	return (vfs_node_t*)out;
}



static vfs_node_t* _fuse_create(void){
	fuse_vfs_node_t* out=omm_alloc(_fuse_vfs_node_allocator);
	out->node_id=0;
	out->file_handle=0;
	out->data_valid_end_time=0;
	out->name_valid_end_time=0;
	out->size=0;
	return (vfs_node_t*)out;
}



static void _fuse_delete(vfs_node_t* node){
	omm_dealloc(_fuse_vfs_node_allocator,node);
}



static vfs_node_t* _fuse_lookup(vfs_node_t* node,const string_t* name){
	fuse_vfs_node_t* fuse_node=(fuse_vfs_node_t*)node;
	if (!fuse_node->node_id){
		return NULL;
	}
	_check_for_updates(fuse_node);
	fuse_lookup_out_t* fuse_lookup_out=virtio_fs_fuse_lookup(node->fs->extra_data,fuse_node->node_id,name->data,name->length+1);
	if (fuse_lookup_out->header.error){
		amm_dealloc(fuse_lookup_out);
		return NULL;
	}
	vfs_node_t* out=_open_node(node->fs,fuse_lookup_out->nodeid,name);
	((fuse_vfs_node_t*)out)->name_valid_end_time=clock_get_time()+fuse_lookup_out->entry_valid*1000000000ull+fuse_lookup_out->entry_valid_nsec;
	amm_dealloc(fuse_lookup_out);
	return out;
}



static u64 _fuse_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	fuse_vfs_node_t* fuse_node=(fuse_vfs_node_t*)node;
	if (!fuse_node->node_id){
		return 0;
	}
	_check_for_updates(fuse_node);
	fuse_read_out_t* fuse_read_out=amm_alloc(sizeof(fuse_read_out_t)+sizeof(fuse_dirent_t)+256);
_retry_read:
	virtio_fs_fuse_read(node->fs->extra_data,fuse_node->node_id,fuse_node->file_handle,pointer,fuse_read_out,sizeof(fuse_read_out_t)+sizeof(fuse_dirent_t)+256,FUSE_OPCODE_READDIR);
	fuse_dirent_t* dirent=(fuse_dirent_t*)(fuse_read_out->data);
	if (fuse_read_out->header.error||fuse_read_out->header.len<=sizeof(fuse_read_out_t)+sizeof(fuse_dirent_t)){
		amm_dealloc(fuse_read_out);
		return 0;
	}
	pointer=(dirent->namelen?dirent->off:0);
	if (fuse_read_out->header.len){
		if ((dirent->namelen==1&&dirent->name[0]=='.')||(dirent->namelen==2&&dirent->name[0]=='.'&&dirent->name[1]=='.')){
			goto _retry_read;
		}
		*out=smm_alloc(dirent->name,dirent->namelen);
	}
	amm_dealloc(fuse_read_out);
	return pointer;
}



static u64 _fuse_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	fuse_vfs_node_t* fuse_node=(fuse_vfs_node_t*)node;
	if (!fuse_node->node_id){
		return 0;
	}
	_check_for_updates(fuse_node);
	if (offset>=fuse_node->size){
		return 0;
	}
	if (size+offset>fuse_node->size){
		size=fuse_node->size-offset;
	}
	if (!size){
		return 0;
	}
	fuse_read_out_t* fuse_read_out=amm_alloc(sizeof(fuse_read_out_t)+size);
	virtio_fs_fuse_read(node->fs->extra_data,fuse_node->node_id,fuse_node->file_handle,offset,fuse_read_out,sizeof(fuse_read_out_t)+size,((node->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK?FUSE_OPCODE_READLINK:FUSE_OPCODE_READ));
	fuse_read_out->header.len-=sizeof(fuse_read_out_t);
	if (fuse_read_out->header.len<size){
		size=fuse_read_out->header.len;
	}
	memcpy(buffer,fuse_read_out->data,size);
	amm_dealloc(fuse_read_out);
	return size;
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
