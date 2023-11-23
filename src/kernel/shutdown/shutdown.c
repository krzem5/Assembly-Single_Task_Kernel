#include <kernel/lock/spinlock.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/notification/notification.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static notification_dispatcher_t _shutdown_notification_dispatcher;
static omm_allocator_t* _shutdown_function_allocator=NULL;
static shutdown_function_t* _shutdown_root_function=NULL;
static spinlock_t _shutdown_function_lock;



void KERNEL_EARLY_EXEC shutdown_init(void){
	notification_dispatcher_init(&_shutdown_notification_dispatcher);
	_shutdown_function_allocator=omm_init("shutdown_function",sizeof(shutdown_function_t),8,1,pmm_alloc_counter("omm_shutdown_function"));
	spinlock_init(&_shutdown_function_lock);
}



KERNEL_PUBLIC void KERNEL_NORETURN shutdown(_Bool restart){
	scheduler_pause();
	notification_dispatcher_dispatch(&_shutdown_notification_dispatcher,NULL,(restart?NOTIFICATION_TYPE_SHUTDOWN_RESTART:NOTIFICATION_TYPE_SHUTDOWN_POWEROFF));
	if (restart){
		_shutdown_restart();
	}
	for (const shutdown_function_t* function=_shutdown_root_function;function;function=function->next){
		if (function->is_high_priority){
			function->callback();
		}
	}
	for (const shutdown_function_t* function=_shutdown_root_function;function;function=function->next){
		if (!function->is_high_priority){
			function->callback();
		}
	}
	scheduler_task_wait_loop();
}



KERNEL_PUBLIC void shutdown_register_notification_listener(notification_listener_t* listener){
	notification_dispatcher_add_listener(&_shutdown_notification_dispatcher,listener);
}



KERNEL_PUBLIC void shutdown_unregister_notification_listener(notification_listener_t* listener){
	notification_dispatcher_add_listener(&_shutdown_notification_dispatcher,listener);
}



KERNEL_PUBLIC void shutdown_register_shutdown_function(shutdown_function_callback_t callback,_Bool is_high_priority){
	spinlock_acquire_exclusive(&_shutdown_function_lock);
	shutdown_function_t* function=omm_alloc(_shutdown_function_allocator);
	function->next=_shutdown_root_function;
	function->callback=callback;
	function->is_high_priority=is_high_priority;
	_shutdown_root_function=function;
	spinlock_release_exclusive(&_shutdown_function_lock);
}
