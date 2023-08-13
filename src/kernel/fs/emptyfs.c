#include <kernel/drive/drive.h>
#include <kernel/vfs/vfs.h>
#include <kernel/partition/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "emptyfs"



static vfs_node_t* _emptyfs_create(partition_t* fs,_Bool is_directory,const char* name,u8 name_length){
	return NULL;
}



static _Bool _emptyfs_delete(partition_t* fs,vfs_node_t* node){
	return 0;
}



static vfs_node_t* KERNEL_CORE_CODE _emptyfs_get_relative(partition_t* fs,vfs_node_t* node,u8 relative){
	return NULL;
}



static _Bool _emptyfs_set_relative(partition_t* fs,vfs_node_t* node,u8 relative,vfs_node_t* other){
	return 0;
}



static _Bool _emptyfs_move_file(partition_t* fs,vfs_node_t* src_node,vfs_node_t* dst_node){
	return 0;
}



static u64 KERNEL_CORE_CODE _emptyfs_read(partition_t* fs,vfs_node_t* node,u64 offset,u8* buffer,u64 count){
	return fs->drive->read_write(fs->drive->extra_data,offset>>fs->drive->block_size_shift,buffer,count>>fs->drive->block_size_shift)<<fs->drive->block_size_shift;
}



static u64 _emptyfs_write(partition_t* fs,vfs_node_t* node,u64 offset,const u8* buffer,u64 count){
	return fs->drive->read_write(fs->drive->extra_data,(offset>>fs->drive->block_size_shift)|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,count>>fs->drive->block_size_shift)<<fs->drive->block_size_shift;
}



static u64 _emptyfs_get_size(partition_t* fs,vfs_node_t* node){
	return (fs->partition_config.last_block_index-fs->partition_config.first_block_index)<<fs->drive->block_size_shift;
}



static _Bool _emptyfs_set_size(partition_t* fs,vfs_node_t* node,u64 size){
	return 0;
}



static void _emptyfs_flush_cache(partition_t* fs){
	return;
}



static const partition_file_system_config_t KERNEL_CORE_DATA _emptyfs_fs_config={
	sizeof(vfs_node_t),
	PARTITION_FILE_SYSTEM_CONFIG_FLAG_ALIGNED_IO,
	_emptyfs_create,
	_emptyfs_delete,
	_emptyfs_get_relative,
	_emptyfs_set_relative,
	_emptyfs_move_file,
	_emptyfs_read,
	_emptyfs_write,
	_emptyfs_get_size,
	_emptyfs_set_size,
	_emptyfs_flush_cache
};



void KERNEL_CORE_CODE emptyfs_load(const drive_t* drive,const partition_config_t* partition_config){
	LOG_CORE("Loading EmptyFS file system from drive '%s'...",drive->model_number);
	vfs_node_t* root=partition_add(drive,partition_config,&_emptyfs_fs_config,NULL);
	root->type=VFS_NODE_TYPE_FILE;
}
