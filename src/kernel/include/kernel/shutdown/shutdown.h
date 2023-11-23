#ifndef _KERNEL_SHUTDOWN_SHUTDOWN_H_
#define _KERNEL_SHUTDOWN_SHUTDOWN_H_ 1
#include <kernel/notification/notification.h>
#include <kernel/types.h>



typedef void (*shutdown_function_callback_t)(void);



typedef struct _SHUTDOWN_FUNCTION{
	struct _SHUTDOWN_FUNCTION* next;
	shutdown_function_callback_t callback;
	_Bool is_high_priority;
} shutdown_function_t;



void shutdown_init(void);



void KERNEL_NORETURN shutdown(_Bool restart);



void shutdown_register_notification_listener(notification_listener_t* listener);



void shutdown_unregister_notification_listener(notification_listener_t* listener);



void shutdown_register_shutdown_function(shutdown_function_callback_t callback,_Bool is_high_priority);



void KERNEL_NORETURN _shutdown_restart(void);



#endif
