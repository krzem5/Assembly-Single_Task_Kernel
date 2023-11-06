#include <devfs/fs.h>
#include <kernel/drive/drive.h>
#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/vfs/node.h>
#define KERNEL_LOG_NAME "devfs_drive"



void devfs_drive_init(void){
	LOG("Creating drive subsystem...");
	vfs_node_t* root=devfs_create_node(devfs->root,"drive",NULL);
	HANDLE_FOREACH(HANDLE_TYPE_DRIVE){
		handle_acquire(handle);
		drive_t* drive=handle->object;
		char buffer[32];
		format_string(buffer,32,"%s%ud%u",drive->type->name,drive->controller_index,drive->device_index);
		vfs_node_t* node=devfs_create_node(root,buffer,NULL);
		devfs_create_node(node,"serial_number",smm_duplicate(drive->serial_number));
		devfs_create_node(node,"model_number",smm_duplicate(drive->model_number));
		devfs_create_data_node(node,"block_count","%lu",drive->block_count);
		devfs_create_data_node(node,"block_size","%lu",drive->block_size);
		devfs_create_node(node,"partition",NULL);//
		handle_release(handle);
	}
}
