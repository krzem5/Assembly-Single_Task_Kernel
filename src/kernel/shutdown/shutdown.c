#include <kernel/io/io.h>
#include <kernel/notification/notification.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static notification_dispatcher_t _shutdown_notification_dispatcher;
static _Bool _shutdown_notification_dispatcher_initialized=0;



static void _init_notification_dispatcher(void){
	if (_shutdown_notification_dispatcher_initialized){
		return;
	}
	notification_dispatcher_init(&_shutdown_notification_dispatcher);
	_shutdown_notification_dispatcher_initialized=1;
}



KERNEL_PUBLIC void KERNEL_NORETURN shutdown(_Bool restart){
	_init_notification_dispatcher();
	notification_dispatcher_dispatch(&_shutdown_notification_dispatcher,NULL,(restart?NOTIFICATION_TYPE_SHUTDOWN_RESTART:NOTIFICATION_TYPE_SHUTDOWN_POWEROFF));
	if (restart){
		_shutdown_restart();
	}
	io_port_out16(0xb004,0x2000); // QEMU specific
	panic("Unable to perform shutdown");
}



KERNEL_PUBLIC void shutdown_register_notification_listener(notification_listener_t* listener){
	_init_notification_dispatcher();
	notification_dispatcher_add_listener(&_shutdown_notification_dispatcher,listener);
}



KERNEL_PUBLIC void shutdown_unregister_notification_listener(notification_listener_t* listener){
	_init_notification_dispatcher();
	notification_dispatcher_add_listener(&_shutdown_notification_dispatcher,listener);
}
