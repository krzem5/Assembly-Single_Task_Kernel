#include <devfs/fs.h>
#include <dynamicfs/dynamicfs.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/numa/numa.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_numa"



MODULE_POSTINIT(){
	LOG("Creating numa subsystem...");
	if (!numa_node_count){
		WARN("Skipping numa subsystem initialization; no NUMA nodes found");
		return;
	}
	vfs_node_t* root=dynamicfs_create_node(devfs->root,"numa",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u32 i=0;i<numa_node_count;i++){
		char buffer[32];
		format_string(buffer,32,"numa%u",i);
		dynamicfs_create_link_node(devfs->root,buffer,"numa/%s",buffer);
		const numa_node_t* numa_node=numa_nodes+i;
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		vfs_node_t* device_root=dynamicfs_create_node(node,"devices",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		u32 j=0;
		for (const numa_cpu_t* numa_cpu=numa_node->cpus;numa_cpu;numa_cpu=numa_cpu->next){
			format_string(buffer,32,"numa%uc%u",i,j);
			vfs_node_t* cpu=dynamicfs_create_node(device_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_data_node(cpu,"id","%u",numa_cpu->apic_id));
			vfs_node_unref(dynamicfs_create_data_node(cpu,"hot_pluggable","0"));
			vfs_node_unref(cpu);
			vfs_node_unref(dynamicfs_create_link_node(devfs->root,buffer,"numa/numa%u/devices/%s",i,buffer));
			j++;
		}
		j=0;
		for (const numa_memory_range_t* numa_memory=numa_node->memory_ranges;numa_memory;numa_memory=numa_memory->next){
			format_string(buffer,32,"numa%um%u",i,j);
			vfs_node_t* memory=dynamicfs_create_node(device_root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
			vfs_node_unref(dynamicfs_create_data_node(memory,"address","%lu",numa_memory->base_address));
			vfs_node_unref(dynamicfs_create_data_node(memory,"length","%lu",numa_memory->length));
			vfs_node_unref(dynamicfs_create_data_node(memory,"hot_pluggable","%u",numa_memory->hot_pluggable));
			vfs_node_unref(memory);
			vfs_node_unref(dynamicfs_create_link_node(devfs->root,buffer,"numa/numa%u/devices/%s",i,buffer));
			j++;
		}
		vfs_node_t* locality_root=dynamicfs_create_node(node,"locality",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		for (j=0;j<numa_node_count;j++){
			format_string(buffer,32,"%u",j);
			vfs_node_unref(dynamicfs_create_data_node(locality_root,buffer,"%u",numa_node_locality_matrix[NUMA_LOCALITY_INDEX(i,j)]));
		}
		vfs_node_unref(node);
		vfs_node_unref(device_root);
	}
	vfs_node_unref(root);
}
