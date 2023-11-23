#ifndef _KERNEL_SHUTDOWN_SHUTDOWN_H_
#define _KERNEL_SHUTDOWN_SHUTDOWN_H_ 1
#include <kernel/notification/notification.h>
#include <kernel/types.h>



void KERNEL_NORETURN shutdown(_Bool restart);



void shutdown_register_notification_listener(notification_listener_t* listener);



void shutdown_unregister_notification_listener(notification_listener_t* listener);



void KERNEL_NORETURN _shutdown_restart(void);



#endif
