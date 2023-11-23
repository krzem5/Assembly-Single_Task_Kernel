#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "lockfs"



static const filesystem_descriptor_config_t _lockfs_filesystem_descriptor_config={
	"lockfs",
	NULL,
	NULL
};



_Bool lockfs_create_fs(void){
	LOG("Creating lockfs filesystem...");
	if (!lock_profiling_type_descriptors||!lock_profiling_data_descriptor_head){
		WARN("No lock profiling data present");
		return 0;
	}
	filesystem_t* fs=dynamicfs_init("/lock",&_lockfs_filesystem_descriptor_config);
	vfs_node_t* type_root=dynamicfs_create_node(fs->root,"type",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	u32 lock_count=0;
	for (;lock_count<LOCK_PROFILING_MAX_LOCK_TYPES;lock_count++){
		if (lock_count&&!(lock_profiling_type_descriptors+lock_count)->func){
			break;
		}
		char buffer[32];
		format_string(buffer,32,"%u",lock_count);
		vfs_node_t* node=dynamicfs_create_node(type_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"location","%s:%u",(lock_profiling_type_descriptors+lock_count)->func,(lock_profiling_type_descriptors+lock_count)->line);
		dynamicfs_create_data_node(node,"name","%s",(lock_profiling_type_descriptors+lock_count)->arg);
	}
	vfs_node_t* data_root=dynamicfs_create_node(fs->root,"data",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	u32 i=0;
	for (const lock_profiling_data_descriptor_t* descriptor=lock_profiling_data_descriptor_head;descriptor;descriptor=descriptor->next){
		char buffer[32];
		format_string(buffer,32,"%u",i);
		vfs_node_t* node=dynamicfs_create_node(data_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		dynamicfs_create_data_node(node,"location","%s:%u",descriptor->func,descriptor->line);
		dynamicfs_create_data_node(node,"name","%s",descriptor->arg);
		for (u32 j=0;j<lock_count;j++){
			char buffer[32];
			format_string(buffer,32,"%u",j);
			vfs_node_t* subnode=dynamicfs_create_node(node,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			dynamicfs_create_node(subnode,"count",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&((descriptor->data+j)->count)));
			dynamicfs_create_node(subnode,"ticks",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&((descriptor->data+j)->ticks)));
			dynamicfs_create_node(subnode,"max_ticks",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(&((descriptor->data+j)->max_ticks)));
		}
		i++;
	}
	return 1;
}
