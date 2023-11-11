#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/log/log.h>
#include <kernel/pipe/pipe.h>
#include <kernel/random/random.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_virtual_file"



static u64 _zero_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	memset(buffer,0,size);
	return size;
}



static u64 _one_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	memset(buffer,0xff,size);
	return size;
}



static u64 _random_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	random_generate(buffer,size);
	return size;
}



void devfs_virtual_file_init(void){
	LOG("Creating virtual file subsystem...");
	dynamicfs_create_node(devfs->root,"zero",VFS_NODE_TYPE_FILE,NULL,_zero_read_callback,NULL);
	dynamicfs_create_node(devfs->root,"one",VFS_NODE_TYPE_FILE,NULL,_one_read_callback,NULL);
	dynamicfs_create_node(devfs->root,"random",VFS_NODE_TYPE_FILE,NULL,_random_read_callback,NULL);
	dynamicfs_create_link_node(devfs->root,"stdin","ser0/in");
	dynamicfs_create_link_node(devfs->root,"stdout","ser0/out");
}
