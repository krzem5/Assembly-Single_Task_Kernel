#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "initramfs"



#define INITRAMFS_HEADER_SIGNATURE 0x007f534654494e49ull

#define INITRAMFS_FLAG_DIRECTORY 1

#define INITRAMFS_ALIGN_SIZE(size) (((size)+7)&0xfffffff8)



typedef struct __attribute__((packed)) _INITRAMFS_HEADER{
	u64 signature;
} initramfs_header_t;



typedef struct __attribute__((packed)) _INITRAMFS_NODE{
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



static pmm_counter_descriptor_t _initramfs_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_initramfs_node");
static omm_allocator_t _initramfs_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("initramfs_node",sizeof(initramfs_vfs_node_t),8,2,&_initramfs_node_omm_pmm_counter);



static filesystem_descriptor_t _initramfs_filesystem_descriptor;



static u64 _initramfs_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		return 0;
	}
	if (offset>=kernel_data.initramfs_size){
		return 0;
	}
	if (offset+count>kernel_data.initramfs_size){
		count=kernel_data.initramfs_size-offset;
	}
	if (!count){
		return 0;
	}
	memcpy(buffer,(void*)(kernel_data.initramfs_address+offset+VMM_HIGHER_HALF_ADDRESS_OFFSET),count);
	return count;
}



static drive_type_t _initramfs_drive_type={
	"INITRAMFS",
	_initramfs_read_write
};



static _Bool _initramfs_init_partitions(drive_t* drive){
	if (!streq(drive->type->name,"INITRAMFS")){
		return 0;
	}
	partition_t* partition=partition_create(drive,"INITRAMFS",0,drive->block_count);
	if (!partition->fs){
		return 0;
	}
	vfs_mount(partition->fs,NULL);
	return 1;
}



static partition_descriptor_t _initramfs_partition_descriptor={
	"INITRAMFS",
	_initramfs_init_partitions
};



static vfs_node_t* _initramfs_create(void){
	initramfs_vfs_node_t* out=omm_alloc(&_initramfs_vfs_node_allocator);
	out->offset=0;
	out->size=0;
	out->data_size=0;
	out->child_count=0;
	out->name_length=0;
	out->flags=0;
	return (vfs_node_t*)out;
}



static void _initramfs_delete(vfs_node_t* node){
	omm_dealloc(&_initramfs_vfs_node_allocator,node);
}



static vfs_node_t* _initramfs_lookup(vfs_node_t* node,const vfs_name_t* name){
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



static u64 _initramfs_iterate(vfs_node_t* node,u64 pointer,vfs_name_t** out){
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
	*out=vfs_name_alloc(buffer,child_node.name_length);
	return pointer+child_node.size;
}



static s64 _initramfs_read(vfs_node_t* node,u64 offset,void* buffer,u64 size){
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



static s64 _initramfs_resize(vfs_node_t* node,s64 size,u32 flags){
	if (!(flags&VFS_NODE_FLAG_RESIZE_RELATIVE)||size){
		return -1;
	}
	return ((initramfs_vfs_node_t*)node)->data_size;
}



static vfs_functions_t _initramfs_functions={
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



static void _initramfs_fs_deinit(filesystem_t* fs){
	panic("_initramfs_fs_deinit");
}



static filesystem_t* _initramfs_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	if (partition->start_lba||!streq(drive->type->name,"INITRAMFS")||drive->block_size!=1){
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
	filesystem_t* out=fs_create(&_initramfs_filesystem_descriptor);
	out->functions=&_initramfs_functions;
	out->partition=partition;
	vfs_name_t* root_name=vfs_name_alloc("<root>",0);
	out->root=vfs_node_create(out,root_name);
	vfs_name_dealloc(root_name);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT|VFS_NODE_TYPE_DIRECTORY;
	((initramfs_vfs_node_t*)(out->root))->offset=sizeof(initramfs_header_t);
	((initramfs_vfs_node_t*)(out->root))->size=node.size;
	((initramfs_vfs_node_t*)(out->root))->data_size=node.data_size;
	((initramfs_vfs_node_t*)(out->root))->child_count=node.child_count;
	((initramfs_vfs_node_t*)(out->root))->name_length=node.name_length;
	((initramfs_vfs_node_t*)(out->root))->flags=node.flags;
	return out;
}



static filesystem_descriptor_t _initramfs_filesystem_descriptor={
	"initramfs",
	_initramfs_fs_deinit,
	_initramfs_fs_load
};



void initramfs_init(void){
	LOG("Loading initramfs...");
	INFO("Address: %p, Size: %v",kernel_data.initramfs_address,kernel_data.initramfs_size);
	fs_register_descriptor(&_initramfs_filesystem_descriptor);
	partition_register_descriptor(&_initramfs_partition_descriptor);
	drive_register_type(&_initramfs_drive_type);
	INFO("Creating virtual drive...");
	drive_config_t config={
		.type=&_initramfs_drive_type,
		.name="initramfs",
		.serial_number="initramfs",
		.model_number="initramfs",
		.block_count=pmm_align_up_address(kernel_data.initramfs_size),
		.block_size=1
	};
	drive_create(&config);
}
