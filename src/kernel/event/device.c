#include <kernel/drive/drive.h>
#include <kernel/event/device.h>
#include <kernel/handle/handle.h>
#include <kernel/notification/notification.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



KERNEL_PUBLIC notification_dispatcher_t* KERNEL_INIT_WRITE event_device_notification_dispatcher=NULL;



KERNEL_EARLY_INIT(){
	event_device_notification_dispatcher=notification_dispatcher_create();
}



void event_dispatch_drive_create_notification(drive_t* drive){
	event_drive_create_notification_data_t data={
		drive->handle.rb_node.key,
		drive->controller_index,
		drive->device_index
	};
	notification_dispatcher_dispatch(event_device_notification_dispatcher,EVENT_DRIVE_CREATE_NOTIFICATION,&data,sizeof(event_drive_create_notification_data_t));
}



void event_dispatch_drive_delete_notification(drive_t* drive){
	event_drive_delete_notification_data_t data={
		drive->handle.rb_node.key,
		drive->controller_index,
		drive->device_index
	};
	notification_dispatcher_dispatch(event_device_notification_dispatcher,EVENT_DRIVE_DELETE_NOTIFICATION,&data,sizeof(event_drive_delete_notification_data_t));
}



void event_dispatch_partition_create_notification(partition_t* partition){
	event_partition_create_notification_data_t data={
		partition->drive->handle.rb_node.key,
		partition->handle.rb_node.key,
		partition->drive->controller_index,
		partition->drive->device_index,
		partition->index
	};
	notification_dispatcher_dispatch(event_device_notification_dispatcher,EVENT_PARTITION_CREATE_NOTIFICATION,&data,sizeof(event_partition_create_notification_data_t));
}



void event_dispatch_partition_delete_notification(partition_t* partition){
	event_partition_delete_notification_data_t data={
		partition->drive->handle.rb_node.key,
		partition->handle.rb_node.key,
		partition->drive->controller_index,
		partition->drive->device_index,
		partition->index
	};
	notification_dispatcher_dispatch(event_device_notification_dispatcher,EVENT_PARTITION_DELETE_NOTIFICATION,&data,sizeof(event_partition_delete_notification_data_t));
}
