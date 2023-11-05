#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/partition/partition.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "devfs_fs"



typedef struct _DEVFS_VFS_NODE{
	vfs_node_t node;
} devfs_vfs_node_t;



static pmm_counter_descriptor_t _devfs_node_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_devfs_node");
static omm_allocator_t _devfs_vfs_node_allocator=OMM_ALLOCATOR_INIT_STRUCT("devfs_node",sizeof(devfs_vfs_node_t),8,2,&_devfs_node_omm_pmm_counter);



static vfs_node_t* _devfs_create(void){
	return omm_alloc(&_devfs_vfs_node_allocator);
}



static const vfs_functions_t _devfs_functions={
	_devfs_create,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};



static void _devfs_fs_deinit(filesystem_t* fs){
	return;
}



static filesystem_t* _devfs_fs_load(partition_t* partition){
	return NULL;
}



static filesystem_descriptor_t _devfs_filesystem_descriptor={
	"devfs",
	_devfs_fs_deinit,
	_devfs_fs_load
};



void devfs_create_fs(void){
	LOG("Creating devfs filesystem...");
	fs_register_descriptor(&_devfs_filesystem_descriptor);
	filesystem_t* devfs=fs_create(&_devfs_filesystem_descriptor);
	devfs->functions=&_devfs_functions;
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	devfs->root=vfs_node_create(devfs,root_name);
	devfs->root->flags|=VFS_NODE_FLAG_VIRTUAL|VFS_NODE_TYPE_DIRECTORY;
	vfs_mount(devfs,"/dev");
}
