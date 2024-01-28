#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "ui_permissions"



static error_t _acl_permission_request_callback(handle_t* handle,process_t* process,u64 flags){
	WARN("Request: %s: %s, %x",process->name->data,handle_get_descriptor(HANDLE_ID_GET_TYPE(handle->rb_node.key))->name,flags);
	return ERROR_DENIED;
}



void ui_permission_backend_init(void){
	LOG("Initializing UI permission backend...");
	acl_request_callback=_acl_permission_request_callback;
}
