#include <dynamicfs/dynamicfs.h>
#include <kernel/cpu/cpu.h>
#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/vfs/node.h>
#include <sysfs/fs.h>
#define KERNEL_LOG_NAME "sysfs_scheduler"



void sysfs_scheduler_init(void){
	LOG("Creating scheduler subsystem...");
	vfs_node_t* root=dynamicfs_create_node(sysfs->root,"scheduler_time",VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
	for (u16 i=0;i<cpu_count;i++){
		char buffer[16];
		format_string(buffer,16,"%u",i);
		vfs_node_t* node=dynamicfs_create_node(root,buffer,VFS_NODE_TYPE_DIRECTORY,NULL,NULL,NULL);
		const scheduler_timers_t* timers=scheduler_get_timers(i);
		dynamicfs_create_node(node,"user",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_USER));
		dynamicfs_create_node(node,"kernel",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_KERNEL));
		dynamicfs_create_node(node,"scheduler",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_SCHEDULER));
		dynamicfs_create_node(node,"none",VFS_NODE_TYPE_FILE,NULL,dynamicfs_integer_read_callback,(void*)(timers->data+SCHEDULER_TIMER_NONE));
	}
}
