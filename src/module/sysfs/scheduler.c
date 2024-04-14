#include <dynamicfs/dynamicfs.h>
#include <kernel/cpu/cpu.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_scheduler"



MODULE_POSTINIT(){
	LOG("Creating scheduler subsystem...");
	vfs_node_t* root=dynamicfs_create_node(sysfs->root,"sched",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u16 i=0;i<cpu_count;i++){
		char buffer[16];
		format_string(buffer,16,"%u",i);
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		vfs_node_t* time=dynamicfs_create_node(node,"time",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		const scheduler_timers_t* timers=scheduler_get_timers(i);
		dynamicfs_create_node(time,"user",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_USER));
		dynamicfs_create_node(time,"kernel",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_KERNEL));
		dynamicfs_create_node(time,"scheduler",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_SCHEDULER));
		dynamicfs_create_node(time,"none",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_NONE));
	}
}
