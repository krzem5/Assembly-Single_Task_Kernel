#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



#define BLOCK_SIZE_SHIFT 9



static fs_node_t* _emptyfs_get_relative(fs_file_system_t* fs,fs_node_t* node,u8 relative){
	return NULL;
}



static _Bool _emptyfs_set_relative(fs_file_system_t* fs,fs_node_t* node,u8 relative,fs_node_t* other){
	return 0;
}



static u64 _emptyfs_read(fs_file_system_t* fs,fs_node_t* node,u64 offset,u8* buffer,u64 count){
	return fs->drive->read_write(fs->drive->extra_data,offset>>BLOCK_SIZE_SHIFT,buffer,count>>BLOCK_SIZE_SHIFT)<<BLOCK_SIZE_SHIFT;
}



static u64 _emptyfs_write(fs_file_system_t* fs,fs_node_t* node,u64 offset,const u8* buffer,u64 count){
	return fs->drive->read_write(fs->drive->extra_data,(offset>>BLOCK_SIZE_SHIFT)|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,count>>BLOCK_SIZE_SHIFT)<<BLOCK_SIZE_SHIFT;
}



static u64 _emptyfs_get_size(fs_file_system_t* fs,fs_node_t* node){
	return (fs->partition_config.last_block_index-fs->partition_config.first_block_index)<<BLOCK_SIZE_SHIFT;
}



static const fs_file_system_config_t _emptyfs_fs_config={
	sizeof(fs_node_t),
	_emptyfs_get_relative,
	_emptyfs_set_relative,
	_emptyfs_read,
	_emptyfs_write,
	_emptyfs_get_size
};



void fs_emptyfs_load(drive_t* drive,const fs_partition_config_t* partition_config){
	LOG("Loading EmptyFS file system from drive '%s'...",drive->model_number);
	if (drive->block_size!=(1<<BLOCK_SIZE_SHIFT)){
		WARN("Unimplemented: drive->block_size!=%u",1<<BLOCK_SIZE_SHIFT);
	}
	fs_node_t* root=fs_create_file_system(drive,partition_config,&_emptyfs_fs_config);
	root->type=FS_NODE_TYPE_FILE;
}
