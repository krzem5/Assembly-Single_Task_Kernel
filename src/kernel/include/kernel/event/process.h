#ifndef _KERNEL_EVENT_PROCESS_H_
#define _KERNEL_EVENT_PROCESS_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>



#define EVENT_PROCESS_CREATE_NOTIFICATION 0x00000001
#define EVENT_PROCESS_TERMINATE_NOTIFICATION 0x00000002
#define EVENT_PROCESS_DELETE_NOTIFICATION 0x00000003
#define EVENT_THREAD_CREATE_NOTIFICATION 0x00000004
#define EVENT_THREAD_TERMINATE_NOTIFICATION 0x00000005
#define EVENT_THREAD_DELETE_NOTIFICATION 0x00000006



typedef struct _EVENT_PROCESS_CREATE_NOTIFICATION_DATA{
	handle_id_t process_handle;
} event_process_create_notification_data_t;



typedef struct _EVENT_PROCESS_TERMINATE_NOTIFICATION_DATA{
	handle_id_t process_handle;
} event_process_terminate_notification_data_t;



typedef struct _EVENT_PROCESS_DELETE_NOTIFICATION_DATA{
	handle_id_t process_handle;
} event_process_delete_notification_data_t;



typedef struct _EVENT_THREAD_CREATE_NOTIFICATION_DATA{
	handle_id_t process_handle;
	handle_id_t thread_handle;
} event_thread_create_notification_data_t;



typedef struct _EVENT_THREAD_TERMINATE_NOTIFICATION_DATA{
	handle_id_t process_handle;
	handle_id_t thread_handle;
} event_thread_terminate_notification_data_t;



typedef struct _EVENT_THREAD_DELETE_NOTIFICATION_DATA{
	handle_id_t process_handle;
	handle_id_t thread_handle;
} event_thread_delete_notification_data_t;



extern notification_dispatcher_t* event_process_notification_dispatcher;



void event_dispatch_process_create_notification(process_t* process);



void event_dispatch_process_terminate_notification(process_t* process);



void event_dispatch_process_delete_notification(process_t* process);



void event_dispatch_thread_create_notification(thread_t* thread);



void event_dispatch_thread_terminate_notification(thread_t* thread);



void event_dispatch_thread_delete_notification(thread_t* thread);



#endif
