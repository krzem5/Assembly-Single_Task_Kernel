#include <iso9660/structures.h>
#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "iso9660_fs"



typedef struct _ISO9660_VFS_NODE{
	vfs_node_t node;
	u64 current_offset;
	u32 data_offset;
	u32 data_length;
} iso9660_vfs_node_t;



static omm_allocator_t* _iso9660_vfs_node_allocator=NULL;
static filesystem_descriptor_t* _iso9660_filesystem_descriptor=NULL;



static vfs_node_t* _iso9660_create(void){
	iso9660_vfs_node_t* out=omm_alloc(_iso9660_vfs_node_allocator);
	out->current_offset=0xffffffffffffffffull;
	out->data_offset=0;
	out->data_length=0;
	return (vfs_node_t*)out;
}



static void _iso9660_delete(vfs_node_t* node){
	omm_dealloc(_iso9660_vfs_node_allocator,node);
}



static vfs_node_t* _iso9660_lookup(vfs_node_t* node,const string_t* name){
	iso9660_vfs_node_t* iso9660_node=(iso9660_vfs_node_t*)node;
	drive_t* drive=node->fs->partition->drive;
	u32 data_offset=iso9660_node->data_offset;
	if (!data_offset){
		return NULL;
	}
	u32 data_length=iso9660_node->data_length;
	u16 buffer_space=0;
	iso9660_directory_t* directory=NULL;
	u8 buffer[2048];
	while (data_length){
		if (buffer_space&&!directory->length){
			data_length-=buffer_space;
			buffer_space=0;
		}
		if (!buffer_space){
			directory=(iso9660_directory_t*)buffer;
			if (drive_read(drive,data_offset,buffer,1)!=1){
				return NULL;
			}
			buffer_space=2048;
			data_offset++;
			continue; // required to break out of the loop after the last directory entry
		}
		if (directory->length>buffer_space){
			WARN("Unimplemented (directory entry crosses sector boundary)");
			return NULL;
		}
		if ((directory->identifier_length==1&&directory->identifier[0]<2)||(directory->flags&ISO9660_DIRECTORY_FLAG_ASSOCIATED_FILE)){
			goto _skip_directory_entry;
		}
		u8 name_length=directory->identifier_length;
		if (!(directory->flags&ISO9660_DIRECTORY_FLAG_DIRECTOR)){
			name_length-=2;
		}
		if (name_length!=name->length){
			goto _skip_directory_entry;
		}
		for (u8 i=0;i<name_length;i++){
			if (directory->identifier[i]+((directory->identifier[i]>64&&directory->identifier[i]<91)<<5)!=name->data[i]){
				goto _skip_directory_entry;
			}
		}
		vfs_node_t* out=vfs_node_create(node->fs,name);
		out->flags|=((directory->flags&ISO9660_DIRECTORY_FLAG_DIRECTOR)?VFS_NODE_TYPE_DIRECTORY:VFS_NODE_TYPE_FILE);
		((iso9660_vfs_node_t*)out)->current_offset=(iso9660_node->data_offset<<11)+iso9660_node->data_length-data_length;
		((iso9660_vfs_node_t*)out)->data_offset=directory->lba;
		((iso9660_vfs_node_t*)out)->data_length=directory->data_length;
		return out;
_skip_directory_entry:
		data_length-=directory->length;
		buffer_space-=directory->length;
		directory=(iso9660_directory_t*)(buffer+2048-buffer_space);
	}
	return NULL;
}



static u64 _iso9660_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	panic("_iso9660_iterate");
}



static u64 _iso9660_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	iso9660_vfs_node_t* iso9660_node=(iso9660_vfs_node_t*)node;
	drive_t* drive=node->fs->partition->drive;
	if (size+offset>iso9660_node->data_length){
		size=iso9660_node->data_length-offset;
	}
	if (!size){
		return 0;
	}
	u64 out=size;
	u64 block_index=offset>>11;
	u16 padding=offset&2047;
	if (padding){
		u8 disk_buffer[2048];
		if (drive_read(drive,iso9660_node->data_offset+block_index,disk_buffer,1)!=1){
			return 0;
		}
		mem_copy(buffer,disk_buffer+padding,2048-padding);
		block_index++;
		buffer+=2048-padding;
		size-=2048-padding;
		offset&=0xfffff800;
	}
	if (drive_read(drive,iso9660_node->data_offset+block_index,buffer,size>>11)!=(size>>11)){
		return 0;
	}
	block_index+=size>>11;
	buffer+=size&0xfffff800;
	padding=size&2047;
	if (padding){
		u8 disk_buffer[2048];
		if (drive_read(drive,iso9660_node->data_offset+block_index,disk_buffer,1)!=1){
			return 0;
		}
		mem_copy(buffer,disk_buffer,padding);
	}
	return out;
}



static u64 _iso9660_resize(vfs_node_t* node,s64 size,u32 flags){
	if (!(flags&VFS_NODE_FLAG_RESIZE_RELATIVE)||size){
		return -1;
	}
	return ((iso9660_vfs_node_t*)node)->data_length;
}



static const vfs_functions_t _iso9660_functions={
	_iso9660_create,
	_iso9660_delete,
	_iso9660_lookup,
	_iso9660_iterate,
	NULL,
	NULL,
	_iso9660_read,
	NULL,
	_iso9660_resize,
	NULL
};



static void _iso9660_fs_deinit(filesystem_t* fs){
	panic("_iso9660_fs_deinit");
}



static filesystem_t* _iso9660_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	if (partition->start_lba||(!str_equal(drive->type->name,"ATA")&&!str_equal(drive->type->name,"ATAPI"))||drive->block_size!=2048){
		return NULL;
	}
	u32 directory_lba=0;
	u32 directory_data_length=0;
	u64 block_index=16;
	u8 buffer[2048];
	while (1){
		if (drive_read(drive,block_index,buffer,1)!=1){
			return NULL;
		}
		iso9660_volume_descriptor_t* volume_descriptor=(iso9660_volume_descriptor_t*)buffer;
		if (volume_descriptor->identifier[0]!='C'||volume_descriptor->identifier[1]!='D'||volume_descriptor->identifier[2]!='0'||volume_descriptor->identifier[3]!='0'||volume_descriptor->identifier[4]!='1'||volume_descriptor->version!=1){
			return NULL;
		}
		switch (volume_descriptor->type){
			case 1:
				directory_lba=volume_descriptor->primary_volume_descriptor.directory_lba;
				directory_data_length=volume_descriptor->primary_volume_descriptor.directory_data_length;
				goto _directory_lba_found;
			case 255:
				return NULL;
		}
		block_index++;
	}
_directory_lba_found:
	filesystem_t* out=fs_create(_iso9660_filesystem_descriptor);
	out->functions=&_iso9660_functions;
	out->partition=partition;
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=vfs_node_create(out,root_name);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT|VFS_NODE_TYPE_DIRECTORY;
	((iso9660_vfs_node_t*)(out->root))->data_offset=directory_lba;
	((iso9660_vfs_node_t*)(out->root))->data_length=directory_data_length;
	return out;
}



static const filesystem_descriptor_config_t _iso9660_filesystem_descriptor_config={
	"iso9660",
	_iso9660_fs_deinit,
	_iso9660_fs_load
};



void iso9660_register_fs(void){
	_iso9660_filesystem_descriptor=fs_register_descriptor(&_iso9660_filesystem_descriptor_config);
	_iso9660_vfs_node_allocator=omm_init("iso9660_node",sizeof(iso9660_vfs_node_t),8,4,pmm_alloc_counter("omm_iso9660_node"));
	spinlock_init(&(_iso9660_vfs_node_allocator->lock));
}
