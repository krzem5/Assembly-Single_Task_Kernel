#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



static fs_node_t* _emptyfs_create(fs_file_system_t* fs,_Bool is_directory,const char* name,u8 name_length){
	return NULL;
}



static _Bool _emptyfs_delete(fs_file_system_t* fs,fs_node_t* node){
	return 0;
}



static fs_node_t* _emptyfs_get_relative(fs_file_system_t* fs,fs_node_t* node,u8 relative){
	return NULL;
}



static _Bool _emptyfs_set_relative(fs_file_system_t* fs,fs_node_t* node,u8 relative,fs_node_t* other){
	return 0;
}



static u64 _emptyfs_read(fs_file_system_t* fs,fs_node_t* node,u64 offset,u8* buffer,u64 count){
	return fs->drive->read_write(fs->drive->extra_data,offset>>fs->drive->block_size_shift,buffer,count>>fs->drive->block_size_shift)<<fs->drive->block_size_shift;
}



static u64 _emptyfs_write(fs_file_system_t* fs,fs_node_t* node,u64 offset,const u8* buffer,u64 count){
	return fs->drive->read_write(fs->drive->extra_data,(offset>>fs->drive->block_size_shift)|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,count>>fs->drive->block_size_shift)<<fs->drive->block_size_shift;
}



static u64 _emptyfs_get_size(fs_file_system_t* fs,fs_node_t* node){
	return (fs->partition_config.last_block_index-fs->partition_config.first_block_index)<<fs->drive->block_size_shift;
}



static void _emptyfs_flush_cache(fs_file_system_t* fs){
	return;
}



static const fs_file_system_config_t _emptyfs_fs_config={
	sizeof(fs_node_t),
	_emptyfs_create,
	_emptyfs_delete,
	_emptyfs_get_relative,
	_emptyfs_set_relative,
	_emptyfs_read,
	_emptyfs_write,
	_emptyfs_get_size,
	_emptyfs_flush_cache
};



void emptyfs_load(const drive_t* drive,const fs_partition_config_t* partition_config){
	LOG("Loading EmptyFS file system from drive '%s'...",drive->model_number);
	fs_node_t* root=fs_create_file_system(drive,partition_config,&_emptyfs_fs_config,NULL);
	root->type=FS_NODE_TYPE_FILE;
}
