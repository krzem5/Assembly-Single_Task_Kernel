#include <common/kfs2/api.h>
#include <common/kfs2/structures.h>
#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/keyring/master_key.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "kfs2"



typedef struct _KFS2_VFS_NODE{
	vfs_node_t node;
	kfs2_node_t kfs2_node;
} kfs2_vfs_node_t;



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kfs2_buffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _kfs2_vfs_node_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _kfs2_filesystem_allocator=NULL;
static filesystem_descriptor_t* KERNEL_INIT_WRITE _kfs2_filesystem_descriptor=NULL;



static void* _alloc_page(u64 count){
	return (void*)(pmm_alloc(count,_kfs2_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



static void _dealloc_page(void* ptr,u64 count){
	pmm_dealloc(((u64)ptr)-VMM_HIGHER_HALF_ADDRESS_OFFSET,count,_kfs2_buffer_pmm_counter);
}



static vfs_node_t* _create_node_from_kfs_node(filesystem_t* fs,const string_t* name,kfs2_node_t* kfs2_node){
	kfs2_vfs_node_t* out=(kfs2_vfs_node_t*)vfs_node_create(fs,NULL,name,0);
	if ((kfs2_node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_DIRECTORY){
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY;
	}
	else if ((kfs2_node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_LINK){
		out->node.flags|=VFS_NODE_TYPE_LINK;
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_FILE;
	}
	out->node.flags|=((kfs2_node->flags&KFS2_INODE_PERMISSION_MASK)>>KFS2_INODE_PERMISSION_SHIFT)<<VFS_NODE_PERMISSION_SHIFT;
	out->node.time_access=kfs2_node->time_access;
	out->node.time_modify=kfs2_node->time_modify;
	out->node.time_change=kfs2_node->time_change;
	out->node.time_birth=kfs2_node->time_birth;
	out->node.gid=kfs2_node->gid;
	out->node.uid=kfs2_node->uid;
	out->kfs2_node=*kfs2_node;
	return (vfs_node_t*)out;
}



static vfs_node_t* _kfs2_create(vfs_node_t* parent,const string_t* name,u32 flags){
	if (!(flags&VFS_NODE_FLAG_CREATE)){
		kfs2_vfs_node_t* out=omm_alloc(_kfs2_vfs_node_allocator);
		out->kfs2_node.inode=0xffffffff;
		return (vfs_node_t*)out;
	}
	u32 kfs2_flags=0;
	if ((flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		kfs2_flags|=KFS2_INODE_TYPE_DIRECTORY;
	}
	else if ((flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		kfs2_flags|=KFS2_INODE_TYPE_LINK;
	}
	else{
		kfs2_flags|=KFS2_INODE_TYPE_FILE;
	}
	kfs2_node_t ret;
	if (!kfs2_node_create(parent->fs->extra_data,&(((kfs2_vfs_node_t*)parent)->kfs2_node),name->data,name->length,kfs2_flags,&ret)){
		return NULL;
	}
	return _create_node_from_kfs_node(parent->fs,name,&ret);
}



static void _kfs2_delete(vfs_node_t* node){
	omm_dealloc(_kfs2_vfs_node_allocator,node);
}



static vfs_node_t* _kfs2_lookup(vfs_node_t* node,const string_t* name){
	kfs2_node_t ret;
	if (!kfs2_node_lookup(node->fs->extra_data,&(((kfs2_vfs_node_t*)node)->kfs2_node),name->data,name->length,&ret)){
		return NULL;
	}
	return _create_node_from_kfs_node(node->fs,name,&ret);
}



static u64 _kfs2_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	char buffer[255];
	u32 buffer_length=sizeof(buffer);
	pointer=kfs2_node_iterate(node->fs->extra_data,&(((kfs2_vfs_node_t*)node)->kfs2_node),pointer,buffer,&buffer_length);
	if (pointer){
		*out=smm_alloc(buffer,buffer_length);
	}
	return pointer;
}



static bool _kfs2_link(vfs_node_t* node,vfs_node_t* parent){
	if (node->fs->extra_data!=parent->fs->extra_data){
		panic("Cross-filesystem file linking is not supported");
	}
	return kfs2_node_link(node->fs->extra_data,&(((kfs2_vfs_node_t*)parent)->kfs2_node),&(((kfs2_vfs_node_t*)node)->kfs2_node),node->name->data,node->name->length);
}



static bool _kfs2_unlink(vfs_node_t* node,vfs_node_t* parent){
	if (node->fs->extra_data!=parent->fs->extra_data){
		return 0;
	}
	return kfs2_node_unlink(node->fs->extra_data,&(((kfs2_vfs_node_t*)parent)->kfs2_node),&(((kfs2_vfs_node_t*)node)->kfs2_node),node->name->data,node->name->length);
}



static u64 _kfs2_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	return kfs2_node_read(node->fs->extra_data,&(((kfs2_vfs_node_t*)node)->kfs2_node),offset,buffer,size);
}



static u64 _kfs2_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	return kfs2_node_write(node->fs->extra_data,&(((kfs2_vfs_node_t*)node)->kfs2_node),offset,buffer,size);
}



static u64 _kfs2_resize(vfs_node_t* node,s64 size,u32 flags){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node.inode==0xffffffff){
		return 0;
	}
	if (flags&VFS_NODE_FLAG_RESIZE_RELATIVE){
		if (!size){
			return kfs2_node->kfs2_node.size;
		}
		size+=kfs2_node->kfs2_node.size;
	}
	return kfs2_node_resize(node->fs->extra_data,&(kfs2_node->kfs2_node),(size<0?0:size));
}



static void _kfs2_flush(vfs_node_t* node){
	if (!(node->flags&VFS_NODE_FLAG_DIRTY)){
		return;
	}
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	kfs2_node->kfs2_node.flags&=~(KFS2_INODE_TYPE_MASK|KFS2_INODE_PERMISSION_MASK);
	if ((kfs2_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_FILE){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_FILE;
	}
	else if ((kfs2_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_DIRECTORY;
	}
	else if ((kfs2_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_LINK;
	}
	else{
		panic("_kfs2_flush: invalid node type");
	}
	kfs2_node->kfs2_node.flags|=((kfs2_node->node.flags&VFS_NODE_PERMISSION_MASK)>>VFS_NODE_PERMISSION_SHIFT)<<KFS2_INODE_PERMISSION_SHIFT;
	kfs2_node->kfs2_node.time_access=kfs2_node->node.time_access;
	kfs2_node->kfs2_node.time_modify=kfs2_node->node.time_modify;
	kfs2_node->kfs2_node.time_change=kfs2_node->node.time_change;
	kfs2_node->kfs2_node.time_birth=kfs2_node->node.time_birth;
	kfs2_node->kfs2_node.gid=kfs2_node->node.gid;
	kfs2_node->kfs2_node.uid=kfs2_node->node.uid;
	kfs2_node_flush(kfs2_node->node.fs->extra_data,&(kfs2_node->kfs2_node));
	node->flags&=~VFS_NODE_FLAG_DIRTY;
}



static const vfs_functions_t _kfs2_functions={
	_kfs2_create,
	_kfs2_delete,
	_kfs2_lookup,
	_kfs2_iterate,
	_kfs2_link,
	_kfs2_unlink,
	_kfs2_read,
	_kfs2_write,
	_kfs2_resize,
	_kfs2_flush
};



static void _kfs2_fs_deinit(filesystem_t* fs){
	omm_dealloc(_kfs2_filesystem_allocator,fs->extra_data);
	panic("_kfs2_deinit_callback");
}



static filesystem_t* _kfs2_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	kfs2_filesystem_t* extra_data=omm_alloc(_kfs2_filesystem_allocator);
	kfs2_filesystem_config_t config={
		drive,
		(kfs2_filesystem_block_read_callback_t)drive_read,
		(kfs2_filesystem_block_write_callback_t)drive_write,
		_alloc_page,
		_dealloc_page,
		drive->block_size,
		partition->start_lba,
		partition->end_lba,
	};
	if (!kfs2_filesystem_init(&config,extra_data)){
		omm_dealloc(_kfs2_filesystem_allocator,extra_data);
		return NULL;
	}
	filesystem_t* out=fs_create(_kfs2_filesystem_descriptor);
	out->functions=&_kfs2_functions;
	out->partition=partition;
	out->extra_data=extra_data;
	kfs2_node_t root_node;
	kfs2_filesystem_get_root(extra_data,&root_node);
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=_create_node_from_kfs_node(out,root_name,&root_node);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT;
	mem_copy(out->guid,extra_data->root_block.uuid,16);
	return out;
}



static void _kfs2_fs_mount(filesystem_t* fs,const char* path){
	if (!path||!str_equal(path,"/")){
		return;
	}
	kfs2_filesystem_t* extra_data=fs->extra_data;
	keyring_master_key_get_encrypted(extra_data->root_block.master_key,sizeof(extra_data->root_block.master_key));
	kfs2_filesystem_flush_root_block(extra_data);
}



static bool _kfs2_fs_format(partition_t* partition){
	panic("_kfs2_fs_format");
}



static const filesystem_descriptor_config_t _kfs2_filesystem_descriptor_config={
	"kfs2",
	_kfs2_fs_deinit,
	_kfs2_fs_load,
	_kfs2_fs_mount,
	_kfs2_fs_format
};



MODULE_INIT(){
	_kfs2_buffer_pmm_counter=pmm_alloc_counter("kfs2_buffer");
	_kfs2_vfs_node_allocator=omm_init("kfs2_node",sizeof(kfs2_vfs_node_t),8,4);
	spinlock_init(&(_kfs2_vfs_node_allocator->lock));
	_kfs2_filesystem_allocator=omm_init("kfs2_filesystem",sizeof(kfs2_filesystem_t),8,1);
	spinlock_init(&(_kfs2_filesystem_allocator->lock));
}



MODULE_POSTINIT(){
	_kfs2_filesystem_descriptor=fs_register_descriptor(&_kfs2_filesystem_descriptor_config);
}
