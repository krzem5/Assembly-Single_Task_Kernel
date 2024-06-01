#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/pipe/pipe.h>
#include <kernel/random/random.h>
#include <kernel/util/memory.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_virtual_file"



static u64 _zero_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	mem_fill(buffer,size,0);
	return size;
}



static u64 _one_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	mem_fill(buffer,size,0xff);
	return size;
}



static u64 _random_read_callback(void* ctx,u64 offset,void* buffer,u64 size){
	if (!buffer){
		return 0;
	}
	random_generate(buffer,size);
	return size;
}



MODULE_POSTINIT(){
	LOG("Creating virtual file subsystem...");
	vfs_node_unref(dynamicfs_create_node(devfs->root,"zero",VFS_NODE_TYPE_FILE,NULL,_zero_read_callback,NULL));
	vfs_node_unref(dynamicfs_create_node(devfs->root,"one",VFS_NODE_TYPE_FILE,NULL,_one_read_callback,NULL));
	vfs_node_unref(dynamicfs_create_node(devfs->root,"random",VFS_NODE_TYPE_FILE,NULL,_random_read_callback,NULL));
}
