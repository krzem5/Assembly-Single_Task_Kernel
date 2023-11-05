#include <devfs/fs.h>
#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_block"



void devfs_drive_init(void){
	LOG("Creating drive subsystem...");
	vfs_node_t* root=devfs_create_node(devfs->root,"drive",NULL);
	HANDLE_FOREACH(HANDLE_TYPE_DRIVE){
		handle_acquire(handle);
		drive_t* drive=handle->object;
		vfs_node_t* node=devfs_create_node(root,drive->name,NULL);
		devfs_create_data_node(node,"name","%s",drive->name);
		devfs_create_data_node(node,"serial_number","%s",drive->serial_number);
		devfs_create_data_node(node,"model_number","%s",drive->model_number);
		devfs_create_data_node(node,"block_count","%lu",drive->block_count);
		devfs_create_data_node(node,"block_size","%lu",drive->block_size);
		handle_release(handle);
	}
}
