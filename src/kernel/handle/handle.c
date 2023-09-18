#include <kernel/handle/handle.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "handle"



static lock_t _handle_global_lock=LOCK_INIT_STRUCT;
static u64 _handle_next_id=1;



void handle_new(void* object,u8 type,handle_t* out){
	out->type=type;
	lock_init(&(out->lock));
	lock_acquire_exclusive(&_handle_global_lock);
	out->id=_handle_next_id;
	_handle_next_id++;
	lock_release_exclusive(&_handle_global_lock);
}



void handle_delete(handle_t* handle){
	ERROR("Unimplemented: handle_delete");
}
