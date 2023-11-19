#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "lockfs"



static filesystem_descriptor_t _lockfs_filesystem_descriptor={
	"lockfs",
	NULL,
	NULL
};



_Bool lockfs_create_fs(void){
	LOG("Creating lockfs filesystem...");
	u32 setup_descriptor_count;
	const spinlock_profiling_setup_descriptor_t*const* setup_descriptors=spinlock_profiling_get_setup_descriptors(&setup_descriptor_count);
	u32 descriptor_count;
	const spinlock_profiling_descriptor_t*const* descriptors=spinlock_profiling_get_descriptors(&descriptor_count);
	if (!descriptor_count||!setup_descriptor_count){
		WARN("No lock profiling data present");
		return 0;
	}
	filesystem_t* fs=dynamicfs_init("/lock",&_lockfs_filesystem_descriptor);
	vfs_node_t* setup_root=dynamicfs_create_node(fs->root,"setup",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u32 i=0;i<setup_descriptor_count;i++){
		const spinlock_profiling_setup_descriptor_t* descriptor=setup_descriptors[i];
		char buffer[32];
		format_string(buffer,32,"%u",i+1);
		vfs_node_t* node=dynamicfs_create_node(setup_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"location","%s:%u",descriptor->func,descriptor->line);
	}
	vfs_node_t* data_root=dynamicfs_create_node(fs->root,"data",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u32 i=0;i<descriptor_count;i++){
		const spinlock_profiling_descriptor_t* descriptor=descriptors[i];
		char buffer[32];
		format_string(buffer,32,"%u",i);
		vfs_node_t* node=dynamicfs_create_node(data_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"location","%s:%u",descriptor->func,descriptor->line);
		for (u32 j=0;j<setup_descriptor_count+1;j++){
			char buffer[32];
			format_string(buffer,32,"%u",j);
			vfs_node_t* subnode=dynamicfs_create_node(node,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			dynamicfs_create_node(subnode,"count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,&(((*(descriptor->data))+j)->count));
			dynamicfs_create_node(subnode,"time",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,&(((*(descriptor->data))+j)->time));
		}
	}
	return 1;
}
