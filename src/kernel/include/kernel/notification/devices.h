#ifndef _KERNEL_NOTIFICATION_DEVICE_H_
#define _KERNEL_NOTIFICATION_DEVICE_H_ 1
#include <kernel/drive/drive.h>
#include <kernel/handle/handle.h>
#include <kernel/notification/notification.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>



#define DRIVE_CREATE_NOTIFICATION 0x00000001
#define DRIVE_DELETE_NOTIFICATION 0x00000002
#define PARTITION_CREATE_NOTIFICATION 0x00000003
#define PARTITION_DELETE_NOTIFICATION 0x00000004



typedef struct _DRIVE_CREATE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	u16 controller_index;
	u16 device_index;
} drive_create_notification_data_t;



typedef struct _DRIVE_DELETE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	u16 controller_index;
	u16 device_index;
} drive_delete_notification_data_t;



typedef struct _PARTITION_CREATE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	handle_id_t partition_handle;
	u16 controller_index;
	u16 device_index;
	u32 partition_index;
} drive_create_notification_data_t;



typedef struct _PARTITION_DELETE_NOTIFICATION_DATA{
	handle_id_t drive_handle;
	handle_id_t partition_handle;
	u16 controller_index;
	u16 device_index;
	u32 partition_index;
} drive_delete_notification_data_t;



extern notification_dispatcher_t* device_notification_dispatcher;



void device_dispatch_drive_create_notification(drive_t* drive);



void device_dispatch_drive_delete_notification(drive_t* drive);



void device_dispatch_partition_create_notification(partition_t* partition);



void device_dispatch_partition_delete_notification(partition_t* partition);



#endif
