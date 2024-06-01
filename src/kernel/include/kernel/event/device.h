#ifndef _KERNEL_EVENT_DEVICE_H_
#define _KERNEL_EVENT_DEVICE_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/notification/notification.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>



#define EVENT_DRIVE_CREATE_NOTIFICATION 0x00000001
#define EVENT_DRIVE_DELETE_NOTIFICATION 0x00000002
#define EVENT_PARTITION_CREATE_NOTIFICATION 0x00000003
#define EVENT_PARTITION_DELETE_NOTIFICATION 0x00000004



typedef struct _EVENT_DRIVE_CREATE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	u16 controller_index;
	u16 device_index;
} event_drive_create_notification_data_t;



typedef struct _EVENT_DRIVE_DELETE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	u16 controller_index;
	u16 device_index;
} event_drive_delete_notification_data_t;



typedef struct _EVENT_PARTITION_CREATE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	handle_id_t partition_handle;
	u16 controller_index;
	u16 device_index;
	u32 partition_index;
} event_partition_create_notification_data_t;



typedef struct _EVENT_PARTITION_DELETE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	handle_id_t partition_handle;
	u16 controller_index;
	u16 device_index;
	u32 partition_index;
} event_partition_delete_notification_data_t;



extern notification_dispatcher_t* event_device_notification_dispatcher;



void event_dispatch_drive_create_notification(drive_t* drive);



void event_dispatch_drive_delete_notification(drive_t* drive);



void event_dispatch_partition_create_notification(partition_t* partition);



void event_dispatch_partition_delete_notification(partition_t* partition);



#endif
