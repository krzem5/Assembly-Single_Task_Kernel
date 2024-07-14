#include <common/fat32/api.h>
#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "fat32"



typedef struct _KFS2_VFS_NODE{
	vfs_node_t node;
	fat32_node_t fat32_node;
} fat32_vfs_node_t;



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _fat32_buffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fat32_vfs_node_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _fat32_filesystem_allocator=NULL;
static filesystem_descriptor_t* KERNEL_INIT_WRITE _fat32_filesystem_descriptor=NULL;



static void* _alloc_page(u64 count){
	return (void*)(pmm_alloc(count,_fat32_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



static void _dealloc_page(void* ptr,u64 count){
	pmm_dealloc(((u64)ptr)-VMM_HIGHER_HALF_ADDRESS_OFFSET,count,_fat32_buffer_pmm_counter);
}



static vfs_node_t* _create_node_from_kfs_node(filesystem_t* fs,const string_t* name,fat32_node_t* fat32_node){
	fat32_vfs_node_t* out=(fat32_vfs_node_t*)vfs_node_create(fs,NULL,name,0);
	if (fat32_node->flags&FAT32_NODE_FLAG_DIRECTORY){
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY|(0555<<VFS_NODE_PERMISSION_SHIFT);
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_FILE|(0444<<VFS_NODE_PERMISSION_SHIFT);
	}
	if (!(fat32_node->flags&FAT32_NODE_FLAG_READONLY)){
		out->node.flags|=0222<<VFS_NODE_PERMISSION_SHIFT;
	}
	out->node.gid=0;
	out->node.uid=0;
	out->fat32_node=*fat32_node;
	return (vfs_node_t*)out;
}



static vfs_node_t* _fat32_create(vfs_node_t* parent,const string_t* name,u32 flags){
	fat32_vfs_node_t* out=omm_alloc(_fat32_vfs_node_allocator);
	out->fat32_node.pointer=0;
	out->fat32_node.cluster=0;
	out->fat32_node.size=0;
	return (vfs_node_t*)out;
}



static void _fat32_delete(vfs_node_t* node){
	omm_dealloc(_fat32_vfs_node_allocator,node);
}



static vfs_node_t* _fat32_lookup(vfs_node_t* node,const string_t* name){
	fat32_node_t ret;
	if (!fat32_node_lookup(node->fs->extra_data,&(((fat32_vfs_node_t*)node)->fat32_node),name->data,name->length,&ret)){
		return NULL;
	}
	return _create_node_from_kfs_node(node->fs,name,&ret);
}



static u64 _fat32_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	char buffer[255];
	u32 buffer_length=sizeof(buffer);
	pointer=fat32_node_iterate(node->fs->extra_data,&(((fat32_vfs_node_t*)node)->fat32_node),pointer,buffer,&buffer_length);
	if (pointer){
		*out=smm_alloc(buffer,buffer_length);
	}
	return pointer;
}



static u64 _fat32_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	return fat32_node_read(node->fs->extra_data,&(((fat32_vfs_node_t*)node)->fat32_node),offset,buffer,size);
}



static u64 _fat32_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	fat32_vfs_node_t* fat32_node=(fat32_vfs_node_t*)node;
	if ((flags&VFS_NODE_FLAG_GROW)&&offset+size>fat32_node->fat32_node.size){
		fat32_node_resize(node->fs->extra_data,&(fat32_node->fat32_node),offset+size);
	}
	return fat32_node_write(node->fs->extra_data,&(fat32_node->fat32_node),offset,buffer,size);
}



static u64 _fat32_resize(vfs_node_t* node,s64 size,u32 flags){
	fat32_vfs_node_t* fat32_node=(fat32_vfs_node_t*)node;
	if (flags&VFS_NODE_FLAG_RESIZE_RELATIVE){
		if (!size){
			return fat32_node->fat32_node.size;
		}
		size+=fat32_node->fat32_node.size;
	}
	return fat32_node_resize(node->fs->extra_data,&(fat32_node->fat32_node),(size<0?0:size));
}



static void _fat32_flush(vfs_node_t* node){
	if (!(node->flags&VFS_NODE_FLAG_DIRTY)){
		return;
	}
	fat32_vfs_node_t* fat32_node=(fat32_vfs_node_t*)node;
	if ((fat32_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_FILE){
		fat32_node->fat32_node.flags&=~FAT32_NODE_FLAG_DIRECTORY;
	}
	else if ((fat32_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		fat32_node->fat32_node.flags|=FAT32_NODE_FLAG_DIRECTORY;
	}
	else{
		panic("_fat32_flush: invalid node type");
	}
	fat32_node_flush(fat32_node->node.fs->extra_data,&(fat32_node->fat32_node));
	node->flags&=~VFS_NODE_FLAG_DIRTY;
}



static const vfs_functions_t _fat32_functions={
	_fat32_create,
	_fat32_delete,
	_fat32_lookup,
	_fat32_iterate,
	NULL,
	NULL,
	_fat32_read,
	_fat32_write,
	_fat32_resize,
	_fat32_flush
};



static void _fat32_fs_deinit(filesystem_t* fs){
	omm_dealloc(_fat32_filesystem_allocator,fs->extra_data);
	panic("_fat32_deinit_callback");
}



static filesystem_t* _fat32_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	fat32_filesystem_t* extra_data=omm_alloc(_fat32_filesystem_allocator);
	fat32_filesystem_config_t config={
		drive,
		(fat32_filesystem_block_read_callback_t)drive_read,
		(fat32_filesystem_block_write_callback_t)drive_write,
		_alloc_page,
		_dealloc_page,
		drive->block_size,
		partition->start_lba,
		partition->end_lba,
	};
	if (!fat32_filesystem_init(&config,extra_data)){
		omm_dealloc(_fat32_filesystem_allocator,extra_data);
		return NULL;
	}
	filesystem_t* out=fs_create(_fat32_filesystem_descriptor);
	out->functions=&_fat32_functions;
	out->partition=partition;
	out->extra_data=extra_data;
	fat32_node_t root_node;
	fat32_filesystem_get_root(extra_data,&root_node);
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=_create_node_from_kfs_node(out,root_name,&root_node);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT;
	return out;
}



static bool _fat32_fs_format(partition_t* partition){
	panic("_fat32_fs_format");
}



static const filesystem_descriptor_config_t _fat32_filesystem_descriptor_config={
	"fat32",
	_fat32_fs_deinit,
	_fat32_fs_load,
	NULL,
	_fat32_fs_format
};



MODULE_INIT(){
	_fat32_buffer_pmm_counter=pmm_alloc_counter("fat32.buffer");
	_fat32_vfs_node_allocator=omm_init("fat32.node",sizeof(fat32_vfs_node_t),8,4);
	rwlock_init(&(_fat32_vfs_node_allocator->lock));
	_fat32_filesystem_allocator=omm_init("fat32.filesystem",sizeof(fat32_filesystem_t),8,1);
	rwlock_init(&(_fat32_filesystem_allocator->lock));
}



MODULE_POSTINIT(){
	fs_register_descriptor(&_fat32_filesystem_descriptor_config,&_fat32_filesystem_descriptor);
}
