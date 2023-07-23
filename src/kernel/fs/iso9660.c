#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



// ISO 9660 directory flags
#define ISO9660_DIRECTORY_FLAG_HIDDEN 1
#define ISO9660_DIRECTORY_FLAG_DIRECTOR 2
#define ISO9660_DIRECTORY_FLAG_ASSOCIATED_FILE 4



typedef struct __attribute__((packed)) _ISO9660_DIRECTORY{
	u8 length;
	u8 _padding;
	u32 lba;
	u8 _padding2[4];
	u32 data_length;
	u8 _padding3[11];
	u8 flags;
	u8 file_unit_size;
	u8 gap_size;
	u8 _padding4[4];
	u8 identifier_length;
	char identifier[];
} iso9660_directory_t;



typedef struct _ISO9660_FS_NODE{
	fs_node_t header;
	u64 parent_offset;
	u64 current_offset;
	u32 data_offset;
	u32 data_length;
} iso9660_fs_node_t;



static fs_node_t* _iso9660_create_node_from_directory_entry(const iso9660_fs_node_t* parent,iso9660_directory_t* directory_entry,u64 current_offset){
	for (u8 i=0;i<directory_entry->identifier_length-2;i++){
		directory_entry->identifier[i]+=(directory_entry->identifier[i]>64&&directory_entry->identifier[i]<91)<<5;
	}
	iso9660_fs_node_t* out=fs_alloc_node(parent->header.fs_index,directory_entry->identifier,directory_entry->identifier_length-2);
	out->header.type=((directory_entry->flags&ISO9660_DIRECTORY_FLAG_DIRECTOR)?FS_NODE_TYPE_DIRECTORY:FS_NODE_TYPE_FILE);
	out->header.parent=parent->header.id;
	out->parent_offset=parent->current_offset;
	out->current_offset=current_offset;
	out->data_offset=directory_entry->lba;
	out->data_length=directory_entry->data_length;
	return (fs_node_t*)out;
}



static fs_node_t* _iso9660_get_relative(drive_t* drive,fs_node_t* node,u8 relative){
	u8 buffer[2048];
	const iso9660_fs_node_t* iso9660_node=(const iso9660_fs_node_t*)node;
	if (relative==FS_NODE_RELATIVE_FIRST_CHILD){
		if (!iso9660_node->data_length){
			return NULL;
		}
		u32 data_offset=iso9660_node->data_offset;
		u32 data_length=iso9660_node->data_length;
		u16 buffer_space=0;
		iso9660_directory_t* directory;
		while (data_length){
			if (buffer_space&&!directory->length){
				data_length-=buffer_space;
				buffer_space=0;
			}
			if (!buffer_space){
				directory=(iso9660_directory_t*)buffer;
				if (drive->read_write(drive->extra_data,data_offset,buffer,1)!=1){
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
			fs_node_t* out=_iso9660_create_node_from_directory_entry(iso9660_node,directory,(iso9660_node->data_offset<<11)+iso9660_node->data_length-data_length);
			out->prev_sibling=FS_NODE_ID_EMPTY;
			return (fs_node_t*)out;
_skip_directory_entry:
			data_length-=directory->length;
			buffer_space-=directory->length;
			directory=(iso9660_directory_t*)(buffer+2048-buffer_space);
		}
		return NULL;
	}
	if (relative==FS_NODE_RELATIVE_PARENT){
		ERROR("Unimplemented (_iso9660_get_relative/FS_NODE_RELATIVE_PARENT)");
		return NULL;
	}
	fs_node_t* parent=fs_get_node_relative(node,FS_NODE_RELATIVE_PARENT);
	if (relative==FS_NODE_RELATIVE_NEXT_SIBLING){
		const iso9660_fs_node_t* iso9660_parent=(const iso9660_fs_node_t*)parent;
		u32 offset=iso9660_node->current_offset-(iso9660_parent->data_offset<<11);
		u32 data_offset=iso9660_parent->data_offset+(offset>>11);
		u32 data_length=iso9660_parent->data_length-offset;
		u16 buffer_space=2048-(offset&0x7ff);
		iso9660_directory_t* directory=(iso9660_directory_t*)(buffer+(offset&0x7ff));
		if (drive->read_write(drive->extra_data,data_offset,buffer,1)!=1){
			return NULL;
		}
		data_offset++;
		goto _skip_directory_entry2;
		while (data_length){
			if (buffer_space&&!directory->length){
				data_length-=buffer_space;
				buffer_space=0;
			}
			if (!buffer_space){
				directory=(iso9660_directory_t*)buffer;
				if (drive->read_write(drive->extra_data,data_offset,buffer,1)!=1){
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
				goto _skip_directory_entry2;
			}
			fs_node_t* out=_iso9660_create_node_from_directory_entry(iso9660_parent,directory,(iso9660_parent->data_offset<<11)+iso9660_parent->data_length-data_length);
			out->prev_sibling=FS_NODE_ID_EMPTY;
			return (fs_node_t*)out;
_skip_directory_entry2:
			data_length-=directory->length;
			buffer_space-=directory->length;
			directory=(iso9660_directory_t*)(buffer+2048-buffer_space);
		}
		return NULL;
	}
	ERROR("Unimplemented (_iso9660_get_relative/FS_NODE_RELATIVE_PREV_SIBLING)");
	return NULL;
}



static _Bool _iso9660_set_relative(drive_t* drive,fs_node_t* node,u8 relative,fs_node_t* other){
	return 0;
}



static u64 _iso9660_read(drive_t* drive,fs_node_t* node,u64 offset,u8* buffer,u64 count){
	const iso9660_fs_node_t* iso9660_node=(const iso9660_fs_node_t*)node;
	if (count>iso9660_node->data_length){
		count=iso9660_node->data_length;
	}
	return drive->read_write(drive->extra_data,iso9660_node->data_offset+(offset>>11),buffer,count>>11)<<11;
}



static u64 _iso9660_write(drive_t* drive,fs_node_t* node,u64 offset,const u8* buffer,u64 count){
	return 0;
}



static const fs_file_system_config_t _iso9660_fs_config={
	sizeof(iso9660_fs_node_t),
	_iso9660_get_relative,
	_iso9660_set_relative,
	_iso9660_read,
	_iso9660_write,
};



void fs_iso9660_load(drive_t* drive,const fs_partition_config_t* partition_config,u32 block_index,u32 data_length){
	LOG("Loading ISO 9660 file system from drive '%s'...",drive->model_number);
	INFO("Root block offset: %u, Root block length: %u",block_index,data_length);
	iso9660_fs_node_t* root=fs_create_file_system(drive,partition_config,&_iso9660_fs_config);
	root->parent_offset=0xffffffffffffffffull;
	root->current_offset=0xffffffffffffffffull;
	root->data_offset=block_index;
	root->data_length=data_length;
}
