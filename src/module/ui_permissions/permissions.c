#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/common.h>
#define KERNEL_LOG_NAME "ui_permissions"



typedef struct _UI_PERMISSION_REQUEST{
	struct _UI_PERMISSION_REQUEST* next;
	u64 id;
	char process[256];
	char handle[64];
	u64 flags;
	event_t* event;
	_Bool accepted;
} ui_permission_request_t;



typedef struct _UI_PERMISSION_USER_REQUEST{
	u64 id;
	char process[256];
	char handle[64];
	u64 flags;
} ui_permission_user_request_t;



static omm_allocator_t* _ui_permission_request_allocator=NULL;
static ui_permission_request_t* _ui_permission_request_list_head=NULL;
static ui_permission_request_t* _ui_permission_request_list_tail=NULL;
static u64 _ui_permission_request_list_id=0;
static spinlock_t _ui_permission_request_list_lock;
static event_t* _ui_permission_request_list_event;



static error_t _acl_permission_request_callback(handle_t* handle,process_t* process,u64 flags){
	INFO("Forwarding permission request to the UI...");
	ui_permission_request_t* request=omm_alloc(_ui_permission_request_allocator);
	request->next=NULL;
	strcpy(request->process,process->name->data,sizeof(request->process));
	strcpy(request->handle,handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key))->name,sizeof(request->handle));
	request->flags=flags;
	request->event=event_create();
	request->accepted=0;
	spinlock_acquire_exclusive(&_ui_permission_request_list_lock);
	request->id=_ui_permission_request_list_id;
	_ui_permission_request_list_id++;
	if (_ui_permission_request_list_tail){
		_ui_permission_request_list_tail->next=request;
	}
	else{
		_ui_permission_request_list_head=request;
	}
	_ui_permission_request_list_tail=request;
	spinlock_release_exclusive(&_ui_permission_request_list_lock);
	event_dispatch(_ui_permission_request_list_event,EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
	event_await(request->event);
	event_delete(request->event);
	LOG("Request %s/%s:%u was %s",request->process,request->handle,request->flags,(request->accepted?"accepted":"denied"));
	error_t out=(request->accepted?ERROR_OK:ERROR_DENIED);
	omm_dealloc(_ui_permission_request_allocator,request);
	return out;
}



static error_t _syscall_get_permission_request(ui_permission_user_request_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(ui_permission_user_request_t)){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (syscall_get_user_pointer_max_length(buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(0);
	}
	if (THREAD_DATA->process->handle.rb_node.key!=ui_common_get_process()){
		return ERROR_DENIED;
	}
	event_await(_ui_permission_request_list_event);
	buffer->id=_ui_permission_request_list_head->id;
	memcpy(buffer->process,_ui_permission_request_list_head->process,sizeof(buffer->process));
	memcpy(buffer->handle,_ui_permission_request_list_head->handle,sizeof(buffer->handle));
	buffer->flags=_ui_permission_request_list_head->flags;
	return ERROR_OK;
}



static error_t _syscall_set_permission_request(u64 id,_Bool accepted){
	if (THREAD_DATA->process->handle.rb_node.key!=ui_common_get_process()){
		return ERROR_DENIED;
	}
	spinlock_acquire_exclusive(&_ui_permission_request_list_lock);
	if (!_ui_permission_request_list_head||_ui_permission_request_list_head->id!=id){
		spinlock_release_exclusive(&_ui_permission_request_list_lock);
		return ERROR_DENIED;
	}
	event_t* event=_ui_permission_request_list_head->event;
	_ui_permission_request_list_head->accepted=accepted;
	_ui_permission_request_list_head=_ui_permission_request_list_head->next;
	if (!_ui_permission_request_list_head){
		_ui_permission_request_list_tail=NULL;
		event_set_active(_ui_permission_request_list_event,0,1);
	}
	spinlock_release_exclusive(&_ui_permission_request_list_lock);
	event_dispatch(event,EVENT_DISPATCH_FLAG_BYPASS_ACL);
	return ERROR_OK;
}



static syscall_callback_t const _ui_permission_request_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_permission_request,
	[2]=(syscall_callback_t)_syscall_set_permission_request,
};



void ui_permission_backend_init(void){
	LOG("Initializing UI permission backend...");
	_ui_permission_request_allocator=omm_init("ui_permission_request",sizeof(ui_permission_request_t),8,1,pmm_alloc_counter("omm_ui_permission_request"));
	spinlock_init(&(_ui_permission_request_allocator->lock));
	spinlock_init(&_ui_permission_request_list_lock);
	_ui_permission_request_list_event=event_create();
	acl_request_callback=_acl_permission_request_callback;
	syscall_create_table("ui_permission_request",_ui_permission_request_syscall_functions,sizeof(_ui_permission_request_syscall_functions)/sizeof(syscall_callback_t));
}
