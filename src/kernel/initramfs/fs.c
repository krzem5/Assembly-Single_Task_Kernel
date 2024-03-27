#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "initramfs_fs"



#define INITRAMFS_HEADER_SIGNATURE 0x007f534654494e49ull

#define INITRAMFS_FLAG_DIRECTORY 1

#define INITRAMFS_ALIGN_SIZE(size) (((size)+7)&0xfffffff8)



typedef struct KERNEL_PACKED _INITRAMFS_HEADER{
	u64 signature;
	u8 uuid[16];
} initramfs_header_t;



typedef struct KERNEL_PACKED _INITRAMFS_NODE{
	u32 size;
	u32 data_size;
	u16 child_count;
	u8 flags;
	u8 name_length;
	char name[];
} initramfs_node_t;



typedef struct _INITRAMFS_VFS_NODE{
	vfs_node_t node;
	u32 size;
	u32 offset;
	u32 data_size;
	u16 child_count;
	u8 name_length;
	u8 flags;
} initramfs_vfs_node_t;



static omm_allocator_t* _initramfs_vfs_node_allocator=NULL;
static filesystem_descriptor_t* _initramfs_filesystem_descriptor=NULL;
static filesystem_t* _initramfs_fs=NULL;



static vfs_node_t* _initramfs_create(void){
	initramfs_vfs_node_t* out=omm_alloc(_initramfs_vfs_node_allocator);
	out->offset=0;
	out->size=0;
	out->data_size=0;
	out->child_count=0;
	out->name_length=0;
	out->flags=0;
	return (vfs_node_t*)out;
}



static void _initramfs_delete(vfs_node_t* node){
	omm_dealloc(_initramfs_vfs_node_allocator,node);
}



static vfs_node_t* _initramfs_lookup(vfs_node_t* node,const string_t* name){
	initramfs_vfs_node_t* initramfs_node=(initramfs_vfs_node_t*)node;
	drive_t* drive=node->fs->partition->drive;
	if (!initramfs_node->offset||!(initramfs_node->flags&INITRAMFS_FLAG_DIRECTORY)){
		return NULL;
	}
	u32 offset=initramfs_node->offset+INITRAMFS_ALIGN_SIZE(sizeof(initramfs_node_t)+initramfs_node->name_length)+INITRAMFS_ALIGN_SIZE(initramfs_node->data_size);
	for (u16 i=0;i<initramfs_node->child_count;i++){
		initramfs_node_t child_node;
		if (drive_read(drive,offset,&child_node,sizeof(initramfs_node_t))!=sizeof(initramfs_node_t)){
			return NULL;
		}
		if (child_node.name_length!=name->length){
			goto _skip_entry;
		}
		char buffer[255];
		if (drive_read(drive,offset+sizeof(initramfs_node_t),buffer,child_node.name_length)!=child_node.name_length){
			return NULL;
		}
		for (u8 i=0;i<child_node.name_length;i++){
			if (buffer[i]!=name->data[i]){
				goto _skip_entry;
			}
		}
		vfs_node_t* out=vfs_node_create(node->fs,name);
		out->flags|=((child_node.flags&INITRAMFS_FLAG_DIRECTORY)?VFS_NODE_TYPE_DIRECTORY:VFS_NODE_TYPE_FILE);
		((initramfs_vfs_node_t*)out)->offset=offset;
		((initramfs_vfs_node_t*)out)->size=child_node.size;
		((initramfs_vfs_node_t*)out)->data_size=child_node.data_size;
		((initramfs_vfs_node_t*)out)->child_count=child_node.child_count;
		((initramfs_vfs_node_t*)out)->name_length=child_node.name_length;
		((initramfs_vfs_node_t*)out)->flags=child_node.flags;
		return out;
_skip_entry:
		offset+=child_node.size;
	}
	return NULL;
}



static u64 _initramfs_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	initramfs_vfs_node_t* initramfs_node=(initramfs_vfs_node_t*)node;
	drive_t* drive=node->fs->partition->drive;
	if (!initramfs_node->offset||!(initramfs_node->flags&INITRAMFS_FLAG_DIRECTORY)){
		return 0;
	}
	if (!pointer){
		pointer=initramfs_node->offset+INITRAMFS_ALIGN_SIZE(sizeof(initramfs_node_t)+initramfs_node->name_length)+INITRAMFS_ALIGN_SIZE(initramfs_node->data_size);
	}
	if (pointer>=initramfs_node->offset+initramfs_node->size){
		return 0;
	}
	initramfs_node_t child_node;
	char buffer[255];
	if (drive_read(drive,pointer,&child_node,sizeof(initramfs_node_t))!=sizeof(initramfs_node_t)||drive_read(drive,pointer+sizeof(initramfs_node_t),buffer,child_node.name_length)!=child_node.name_length){
		return 0;
	}
	*out=smm_alloc(buffer,child_node.name_length);
	return pointer+child_node.size;
}



static u64 _initramfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	initramfs_vfs_node_t* initramfs_node=(initramfs_vfs_node_t*)node;
	drive_t* drive=node->fs->partition->drive;
	if (!initramfs_node->offset||offset>=initramfs_node->data_size){
		return 0;
	}
	if (size+offset>initramfs_node->data_size){
		size=initramfs_node->data_size-offset;
	}
	if (!size){
		return 0;
	}
	return drive_read(drive,initramfs_node->offset+INITRAMFS_ALIGN_SIZE(sizeof(initramfs_node_t)+initramfs_node->name_length)+offset,buffer,size);
}



static u64 _initramfs_resize(vfs_node_t* node,s64 size,u32 flags){
	if (!(flags&VFS_NODE_FLAG_RESIZE_RELATIVE)||size){
		return -1;
	}
	return ((initramfs_vfs_node_t*)node)->data_size;
}



static const vfs_functions_t _initramfs_functions={
	_initramfs_create,
	_initramfs_delete,
	_initramfs_lookup,
	_initramfs_iterate,
	NULL,
	NULL,
	_initramfs_read,
	NULL,
	_initramfs_resize,
	NULL
};



static filesystem_t* _initramfs_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	if (partition->start_lba||!str_equal(drive->type->name,"initramfs")||drive->block_size!=1){
		return NULL;
	}
	initramfs_header_t header;
	if (drive_read(drive,0,&header,sizeof(initramfs_header_t))!=sizeof(initramfs_header_t)||header.signature!=INITRAMFS_HEADER_SIGNATURE){
		return NULL;
	}
	initramfs_node_t node;
	if (drive_read(drive,sizeof(initramfs_header_t),&node,sizeof(initramfs_node_t))!=sizeof(initramfs_node_t)){
		return NULL;
	}
	_initramfs_fs=fs_create(_initramfs_filesystem_descriptor);
	_initramfs_fs->functions=&_initramfs_functions;
	_initramfs_fs->partition=partition;
	mem_copy(_initramfs_fs->guid,header.uuid,16);
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	_initramfs_fs->root=vfs_node_create(_initramfs_fs,root_name);
	_initramfs_fs->root->flags|=VFS_NODE_FLAG_PERMANENT|VFS_NODE_TYPE_DIRECTORY;
	((initramfs_vfs_node_t*)(_initramfs_fs->root))->offset=sizeof(initramfs_header_t);
	((initramfs_vfs_node_t*)(_initramfs_fs->root))->size=node.size;
	((initramfs_vfs_node_t*)(_initramfs_fs->root))->data_size=node.data_size;
	((initramfs_vfs_node_t*)(_initramfs_fs->root))->child_count=node.child_count;
	((initramfs_vfs_node_t*)(_initramfs_fs->root))->name_length=node.name_length;
	((initramfs_vfs_node_t*)(_initramfs_fs->root))->flags=node.flags;
	return _initramfs_fs;
}



static const filesystem_descriptor_config_t _initramfs_filesystem_descriptor_config={
	"initramfs",
	NULL,
	_initramfs_fs_load
};



void KERNEL_EARLY_EXEC initramfs_fs_init(void){
	INFO("Registering initramfs filesystem descriptor...");
	_initramfs_vfs_node_allocator=omm_init("initramfs_node",sizeof(initramfs_vfs_node_t),8,2,pmm_alloc_counter("omm_initramfs_node"));
	spinlock_init(&(_initramfs_vfs_node_allocator->lock));
	_initramfs_filesystem_descriptor=fs_register_descriptor(&_initramfs_filesystem_descriptor_config);
}



void initramfs_fs_deinit(void){
	INFO("Unregistering initramfs filesystem descriptor...");
	if (_initramfs_fs){
		handle_release(&(_initramfs_fs->handle));
		_initramfs_fs=NULL;
	}
	if (_initramfs_filesystem_descriptor){
		fs_unregister_descriptor(_initramfs_filesystem_descriptor);
		_initramfs_filesystem_descriptor=NULL;
	}
}
