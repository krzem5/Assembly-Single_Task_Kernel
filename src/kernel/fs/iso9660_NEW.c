#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs2/node.h>
#define KERNEL_LOG_NAME "iso9660"



typedef struct __attribute__((packed)) _ISO9660_VOLUME_DESCRIPTOR{
	u8 type;
	u8 identifier[5];
	u8 version;
	struct __attribute__((packed)){
		u8 _padding[151];
		u32 directory_lba;
		u8 _padding2[4];
		u32 directory_data_length;
	} primary_volume_descriptor;
} iso9660_volume_descriptor_t;



typedef struct _ISO9660_VFS_NODE{
	vfs2_node_t node;
	u64 current_offset;
	u32 data_offset;
	u32 data_length;
} iso9660_vfs_node_t;



PMM_DECLARE_COUNTER(OMM_ISO9660_NODE);



static omm_allocator_t _iso9660_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("iso9660_node",sizeof(iso9660_vfs_node_t),8,4,PMM_COUNTER_OMM_ISO9660_NODE);



extern filesystem_type_t FILESYSTEM_TYPE_ISO9660;



static vfs2_node_t* _iso9660_create(void){
	iso9660_vfs_node_t* out=omm_alloc(&_iso9660_vfs_node_allocator);
	out->current_offset=0xffffffffffffffffull;
	out->data_offset=0;
	out->data_length=0;
	return (vfs2_node_t*)out;
}



static void _iso9660_delete(vfs2_node_t* node){
	omm_dealloc(&_iso9660_vfs_node_allocator,node);
}



static vfs2_node_t* _iso9660_lookup(vfs2_node_t* node,vfs2_node_name_t* name){
	panic("_iso9660_lookup");
}



static _Bool _iso9660_link(vfs2_node_t* node,vfs2_node_t* parent){
	panic("_iso9660_link");
}



static _Bool _iso9660_unlink(vfs2_node_t* node){
	panic("_iso9660_unlink");
}



static s64 _iso9660_read(vfs2_node_t* node,u64 offset,void* buffer,u64 size){
	panic("_iso9660_read");
}



static s64 _iso9660_write(vfs2_node_t* node,u64 offset,const void*,u64 size){
	panic("_iso9660_write");
}



static s64 _iso9660_resize(vfs2_node_t* node,s64 size,u32 flags){
	panic("_iso9660_resize");
}



static void _iso9660_flush(vfs2_node_t* node){
	panic("_iso9660_flush");
}



static vfs2_functions_t _iso9660_functions={
	_iso9660_create,
	_iso9660_delete,
	_iso9660_lookup,
	_iso9660_link,
	_iso9660_unlink,
	_iso9660_read,
	_iso9660_write,
	_iso9660_resize,
	_iso9660_flush
};



static void _iso9660_deinit_callback(filesystem2_t* fs){
	panic("_iso9660_deinit_callback");
}



static filesystem2_t* _iso9660_load_callback(partition2_t* partition){
	if (partition->start_lba||partition->drive->type!=DRIVE_TYPE_ATAPI||partition->drive->block_size!=2048){
		return NULL;
	}
	u32 directory_lba=0;
	u32 directory_data_length=0;
	u64 block_index=16;
	u8 buffer[2048];
	while (1){
		if (partition->drive->read_write(partition->drive->extra_data,block_index,buffer,1)!=1){
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
	filesystem2_t* out=fs_create(FILESYSTEM_TYPE_ISO9660);
	out->functions=&_iso9660_functions;
	out->root=vfs2_node_create(out,"/",1);
	out->root->flags|=VFS2_NODE_FLAG_PERMANENT|VFS2_NODE_TYPE_DIRECTORY;
	((iso9660_vfs_node_t*)(out->root))->data_offset=directory_lba;
	((iso9660_vfs_node_t*)(out->root))->data_length=directory_data_length;
	return out;
}



FILESYSTEM_DECLARE_TYPE(
	ISO9660,
	_iso9660_deinit_callback,
	_iso9660_load_callback
)
