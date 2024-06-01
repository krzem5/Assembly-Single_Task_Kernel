#include <kernel/event/process.h>
#include <kernel/handle/handle.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/notification/notification.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



static bool _event_kernel_process_dispatched=0;

KERNEL_PUBLIC notification_dispatcher_t* KERNEL_INIT_WRITE event_process_notification_dispatcher=NULL;



KERNEL_EARLY_INIT(){
	event_process_notification_dispatcher=notification_dispatcher_create();
}



void event_dispatch_process_create_notification(process_t* process){
	event_process_create_notification_data_t data={
		process->handle.rb_node.key
	};
	notification_dispatcher_dispatch(event_process_notification_dispatcher,EVENT_PROCESS_CREATE_NOTIFICATION,&data,sizeof(event_process_create_notification_data_t));
}



void event_dispatch_process_terminate_notification(process_t* process){
	event_process_terminate_notification_data_t data={
		process->handle.rb_node.key
	};
	notification_dispatcher_dispatch(event_process_notification_dispatcher,EVENT_PROCESS_TERMINATE_NOTIFICATION,&data,sizeof(event_process_terminate_notification_data_t));
}



void event_dispatch_process_delete_notification(process_t* process){
	event_process_delete_notification_data_t data={
		process->handle.rb_node.key
	};
	notification_dispatcher_dispatch(event_process_notification_dispatcher,EVENT_PROCESS_DELETE_NOTIFICATION,&data,sizeof(event_process_delete_notification_data_t));
}



void event_dispatch_thread_create_notification(thread_t* thread){
	if (!_event_kernel_process_dispatched){
		event_dispatch_process_create_notification(process_kernel);
		_event_kernel_process_dispatched=1;
	}
	event_thread_create_notification_data_t data={
		thread->process->handle.rb_node.key,
		thread->handle.rb_node.key
	};
	notification_dispatcher_dispatch(event_process_notification_dispatcher,EVENT_THREAD_CREATE_NOTIFICATION,&data,sizeof(event_thread_create_notification_data_t));
}



void event_dispatch_thread_terminate_notification(thread_t* thread){
	event_thread_terminate_notification_data_t data={
		thread->process->handle.rb_node.key,
		thread->handle.rb_node.key
	};
	notification_dispatcher_dispatch(event_process_notification_dispatcher,EVENT_THREAD_TERMINATE_NOTIFICATION,&data,sizeof(event_thread_terminate_notification_data_t));
}



void event_dispatch_thread_delete_notification(thread_t* thread){
	event_thread_delete_notification_data_t data={
		thread->process->handle.rb_node.key,
		thread->handle.rb_node.key
	};
	notification_dispatcher_dispatch(event_process_notification_dispatcher,EVENT_THREAD_DELETE_NOTIFICATION,&data,sizeof(event_thread_delete_notification_data_t));
}
