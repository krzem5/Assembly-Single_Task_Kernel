#include <kernel/acl/acl.h>
#include <kernel/cpu/cpu.h>
#include <kernel/error/error.h>
#include <kernel/event/process.h>
#include <kernel/format/format.h>
#include <kernel/fpu/fpu.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/thread_list.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



#define MAX_THREAD_CACHE_SIZE 64

#define KERNEL_THREAD_STACK_SIZE 0x200000

#define SET_KERNEL_THREAD_ARG(register) \
	if ((arg_count)){ \
		(arg_count)--; \
		out->reg_state.gpr_state.register=__builtin_va_arg(va,u64); \
	}



static thread_t* _thread_cache_data[MAX_THREAD_CACHE_SIZE];
static u32 _thread_cache_size=0;
static rwlock_t _thread_cache_lock;
static omm_allocator_t* KERNEL_INIT_WRITE _thread_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _thread_fpu_state_allocator=NULL;

KERNEL_PUBLIC handle_type_t KERNEL_INIT_WRITE thread_handle_type=0;



static void _thread_handle_destructor(handle_t* handle){
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	if (thread->state!=THREAD_STATE_TYPE_TERMINATED){
		panic("_thread_handle_destructor: Unterminated thread not referenced");
	}
	event_dispatch_thread_delete_notification(thread);
	lock_profiling_assert_empty(thread);
	process_t* process=thread->process;
	smm_dealloc(thread->name);
	thread->name=NULL;
	event_delete(thread->termination_event);
	signal_thread_state_deinit(&(thread->signal_state));
	if (thread_list_remove(&(process->thread_list),thread)){
		event_dispatch_process_terminate_notification(process);
		event_dispatch(process->event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE|EVENT_DISPATCH_FLAG_BYPASS_ACL);
		handle_release(&(process->handle));
	}
	rwlock_acquire_write(&_thread_cache_lock);
	if (_thread_cache_size<MAX_THREAD_CACHE_SIZE){
		_thread_cache_data[_thread_cache_size]=thread;
		_thread_cache_size++;
		rwlock_release_write(&_thread_cache_lock);
		return;
	}
	rwlock_release_write(&_thread_cache_lock);
	mmap_dealloc_region(process_kernel->mmap,thread->kernel_stack_region);
	omm_dealloc(_thread_fpu_state_allocator,thread->reg_state.fpu_state);
	omm_dealloc(_thread_allocator,thread);
}



static thread_t* _thread_alloc(void){
	rwlock_acquire_write(&_thread_cache_lock);
	if (_thread_cache_size){
		_thread_cache_size--;
		thread_t* out=_thread_cache_data[_thread_cache_size];
		rwlock_release_write(&_thread_cache_lock);
		return out;
	}
	rwlock_release_write(&_thread_cache_lock);
	thread_t* out=omm_alloc(_thread_allocator);
	out->header.current_thread=out;
	rwlock_init(&(out->lock));
	out->name=NULL;
	out->kernel_stack_region=mmap_alloc(process_kernel->mmap,0,KERNEL_THREAD_STACK_SIZE,MMAP_REGION_FLAG_STACK|MMAP_REGION_FLAG_VMM_WRITE,NULL);
	if (!out->kernel_stack_region){
		panic("Unable to reserve thread stack");
	}
	out->reg_state.fpu_state=omm_alloc(_thread_fpu_state_allocator);
	return out;
}



static thread_t* _thread_create(process_t* process){
	if (!_thread_fpu_state_allocator){
		_thread_fpu_state_allocator=omm_init("kernel.thread.fpu_state",fpu_state_size,64,4);
		rwlock_init(&(_thread_fpu_state_allocator->lock));
		rwlock_init(&_thread_cache_lock);
	}
	thread_t* out=_thread_alloc();
	handle_new(thread_handle_type,&(out->handle));
	out->handle.acl=acl_create();
	acl_set(out->handle.acl,process,0,THREAD_ACL_FLAG_TERMINATE|THREAD_ACL_FLAG_CONFIG);
	rwlock_init(&(out->lock));
	out->process=process;
	char buffer[128];
	out->name=smm_alloc(buffer,format_string(buffer,128,"%s.%u",process->name->data,HANDLE_ID_GET_INDEX(out->handle.rb_node.key)));
	out->reg_state.reg_state_not_present=0;
	mem_fill(&(out->reg_state.gpr_state),sizeof(isr_state_t),0);
	fpu_init(out->reg_state.fpu_state);
	out->priority=SCHEDULER_PRIORITY_NORMAL;
	out->state=THREAD_STATE_TYPE_NONE;
	out->event_sequence_id=0;
	format_string(buffer,sizeof(buffer),"%lu",HANDLE_ID_GET_INDEX(out->handle.rb_node.key));
	out->termination_event=event_create("kernel.thread.termination",buffer);
	out->scheduler_load_balancer_queue_index=0;
	out->scheduler_early_yield=0;
	out->scheduler_io_yield=0;
	out->scheduler_forced_queue_index=0;
	out->return_value=NULL;
	signal_thread_state_init(&(out->signal_state));
	lock_profiling_init_lock_stack(out);
	thread_list_add(&(process->thread_list),out);
	return out;
}



KERNEL_EARLY_INIT(){
	LOG("Initializing thread allocator...");
	_thread_allocator=omm_init("kernel.thread",sizeof(thread_t),8,4);
	rwlock_init(&(_thread_allocator->lock));
	thread_handle_type=handle_alloc("kernel.thread",HANDLE_DESCRIPTOR_FLAG_ALLOW_CONTAINER,_thread_handle_destructor);
}



KERNEL_PUBLIC thread_t* thread_create_user_thread(process_t* process,u64 rip,u64 rsp){
	thread_t* out=_thread_create(process);
	out->header.kernel_rsp=out->kernel_stack_region->rb_node.key+KERNEL_THREAD_STACK_SIZE;
	out->reg_state.gpr_state.rip=rip;
	out->reg_state.gpr_state.rsp=rsp;
	out->reg_state.gpr_state.cs=0x23;
	out->reg_state.gpr_state.ds=0x1b;
	out->reg_state.gpr_state.es=0x1b;
	out->reg_state.gpr_state.ss=0x1b;
	out->reg_state.gpr_state.rflags=0x0000000202;
	out->reg_state.fs_gs_state.fs=0;
	out->reg_state.fs_gs_state.gs=0;
	event_dispatch_thread_create_notification(out);
	return out;
}



KERNEL_PUBLIC thread_t* thread_create_kernel_thread(process_t* process,const char* name,void* func,u8 arg_count,...){
	if (arg_count>6){
		panic("thread_create_kernel_thread: too many arguments");
	}
	if (!scheduler_enabled){
		return NULL;
	}
	bool start_thread=!process;
	thread_t* out=_thread_create((process?process:process_kernel));
	if (name){
		smm_dealloc(out->name);
		out->name=smm_alloc(name,0);
	}
	out->reg_state.gpr_state.rip=(u64)_thread_bootstrap_kernel_thread;
	out->reg_state.gpr_state.rax=(u64)func;
	out->reg_state.gpr_state.rsp=out->kernel_stack_region->rb_node.key+KERNEL_THREAD_STACK_SIZE;
	__builtin_va_list va;
	__builtin_va_start(va,arg_count);
	SET_KERNEL_THREAD_ARG(rdi);
	SET_KERNEL_THREAD_ARG(rsi);
	SET_KERNEL_THREAD_ARG(rdx);
	SET_KERNEL_THREAD_ARG(rcx);
	SET_KERNEL_THREAD_ARG(r8);
	SET_KERNEL_THREAD_ARG(r9);
	__builtin_va_end(va);
	out->reg_state.gpr_state.cs=0x08;
	out->reg_state.gpr_state.ds=0x10;
	out->reg_state.gpr_state.es=0x10;
	out->reg_state.gpr_state.ss=0x10;
	out->reg_state.gpr_state.rflags=0x0000000202;
	out->reg_state.fs_gs_state.fs=0;
	out->reg_state.fs_gs_state.gs=(u64)out;
	event_dispatch_thread_create_notification(out);
	if (start_thread){
		scheduler_enqueue_thread(out);
	}
	return out;
}



KERNEL_PUBLIC void thread_delete(thread_t* thread){
	handle_release(&(thread->handle));
}



KERNEL_PUBLIC void KERNEL_NORETURN thread_terminate(void* return_value){
	scheduler_pause();
	thread_t* thread=CPU_HEADER_DATA->current_thread;
	rwlock_acquire_write(&(thread->lock));
	thread->state=THREAD_STATE_TYPE_TERMINATED;
	thread->return_value=return_value;
	if (thread->process->main_thread==thread){
		thread->process->return_value=return_value;
	}
	rwlock_release_write(&(thread->lock));
	event_dispatch_thread_terminate_notification(thread);
	event_dispatch(thread->termination_event,EVENT_DISPATCH_FLAG_DISPATCH_ALL|EVENT_DISPATCH_FLAG_SET_ACTIVE);
	scheduler_yield();
	for (;;);
}



error_t syscall_thread_get_tid(void){
	return THREAD_DATA->handle.rb_node.key;
}



error_t syscall_thread_create(u64 rip,u64 rdi,u64 rsi,u64 rdx,u64 rsp){
	if (!syscall_get_user_pointer_max_length((void*)rip)){
		return ERROR_INVALID_ARGUMENT(0);
	}
	thread_t* thread=thread_create_user_thread(THREAD_DATA->process,rip,rsp);
	thread->reg_state.gpr_state.rdi=rdi;
	thread->reg_state.gpr_state.rsi=rsi;
	thread->reg_state.gpr_state.rdx=rdx;
	scheduler_enqueue_thread(thread);
	return thread->handle.rb_node.key;
}



error_t syscall_thread_stop(handle_id_t thread_handle,KERNEL_USER_POINTER void* return_value){
	if (thread_handle){
		panic("syscall_thread_stop: stop other thread");
	}
	thread_terminate((void*)return_value);
}



error_t syscall_thread_get_priority(handle_id_t thread_handle){
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	if (!(acl_get(thread->handle.acl,THREAD_DATA->process)&THREAD_ACL_FLAG_CONFIG)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	u64 out=thread->priority;
	handle_release(handle);
	return out;
}



error_t syscall_thread_set_priority(handle_id_t thread_handle,u64 priority){
	if (priority<SCHEDULER_PRIORITY_MIN||priority>SCHEDULER_PRIORITY_MAX){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	if (!(acl_get(thread->handle.acl,THREAD_DATA->process)&THREAD_ACL_FLAG_CONFIG)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	if (thread->state==THREAD_STATE_TYPE_TERMINATED){
		handle_release(handle);
		return ERROR_UNSUPPORTED_OPERATION;
	}
	thread->priority=priority;
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_thread_await_events(KERNEL_USER_POINTER const void* events,u64 event_count){
	if (!event_count){
		return ERROR_INVALID_ARGUMENT(1);
	}
	if (event_count*sizeof(handle_id_t)>syscall_get_user_pointer_max_length((const void*)events)){
		return ERROR_INVALID_ARGUMENT(0);
	}
	handle_id_t* buffer=amm_alloc(event_count*sizeof(handle_id_t));
	mem_copy(buffer,(const void*)events,event_count*sizeof(handle_id_t));
	u32 out=event_await_handles(buffer,event_count);
	amm_dealloc(buffer);
	return out;
}



error_t syscall_thread_start(handle_id_t thread_handle){
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	if (!(acl_get(thread->handle.acl,THREAD_DATA->process)&THREAD_ACL_FLAG_CONFIG)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	rwlock_acquire_write(&(thread->lock));
	error_t out=ERROR_UNSUPPORTED_OPERATION;
	if (thread->state==THREAD_STATE_TYPE_NONE){
		out=ERROR_OK;
		rwlock_release_write(&(thread->lock));
		scheduler_enqueue_thread(thread);
	}
	else{
		rwlock_release_write(&(thread->lock));
	}
	handle_release(handle);
	return out;
}



error_t syscall_thread_get_return_value(handle_id_t thread_handle){
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	if (!(acl_get(thread->handle.acl,THREAD_DATA->process)&THREAD_ACL_FLAG_CONFIG)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	u64 out=(u64)(thread->return_value);
	handle_release(handle);
	return out;
}



error_t syscall_thread_iter(handle_id_t process_handle_id,handle_id_t thread_handle_id){
	if (!process_handle_id){
		handle_descriptor_t* thread_handle_descriptor=handle_get_descriptor(thread_handle_type);
		rb_tree_node_t* rb_node=rb_tree_lookup_increasing_node(&(thread_handle_descriptor->tree),(thread_handle_id?thread_handle_id+1:0));
		return (rb_node?rb_node->key:0);
	}
	handle_t* thread_handle=handle_lookup_and_acquire(thread_handle_id,thread_handle_type);
	if (!thread_handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(thread_handle,thread_t,handle);
	if (thread->process->handle.rb_node.key!=process_handle_id){
		handle_release(thread_handle);
		return 0;
	}
	handle_t* process_handle=handle_lookup_and_acquire(process_handle_id,process_handle_type);
	if (!process_handle){
		handle_release(thread_handle);
		return ERROR_INVALID_HANDLE;
	}
	process_t* process=KERNEL_CONTAINEROF(process_handle,process_t,handle);
	rwlock_acquire_read(&(process->thread_list.lock));
	handle_id_t out=(thread->thread_list_next?thread->thread_list_next->handle.rb_node.key:0);
	rwlock_release_read(&(process->thread_list.lock));
	handle_release(process_handle);
	handle_release(thread_handle);
	return out;
}



error_t syscall_thread_query(handle_id_t thread_handle,KERNEL_USER_POINTER thread_query_user_data_t* buffer,u32 buffer_length){
	if (buffer_length<sizeof(thread_query_user_data_t)){
		return ERROR_INVALID_ARGUMENT(2);
	}
	if (syscall_get_user_pointer_max_length((void*)buffer)<buffer_length){
		return ERROR_INVALID_ARGUMENT(1);
	}
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	buffer->pid=thread->process->handle.rb_node.key;
	buffer->tid=thread_handle;
	str_copy(thread->name->data,(char*)(buffer->name),sizeof(buffer->name));
	buffer->state=thread->state;
	buffer->priority=thread->priority;
	buffer->scheduler_priority=thread->scheduler_load_balancer_queue_index;
	handle_release(handle);
	return ERROR_OK;
}



error_t syscall_thread_set_name(handle_id_t thread_handle,KERNEL_USER_POINTER const char* name){
	u64 name_length=syscall_get_string_length((const char*)name);
	if (!name_length||name_length>255){
		return ERROR_INVALID_ARGUMENT(1);
	}
	char buffer[256];
	mem_copy(buffer,(const char*)name,name_length);
	buffer[name_length]=0;
	handle_t* handle=handle_lookup_and_acquire(thread_handle,thread_handle_type);
	if (!handle){
		return ERROR_INVALID_HANDLE;
	}
	thread_t* thread=KERNEL_CONTAINEROF(handle,thread_t,handle);
	if (!(acl_get(thread->handle.acl,THREAD_DATA->process)&THREAD_ACL_FLAG_CONFIG)){
		handle_release(handle);
		return ERROR_DENIED;
	}
	rwlock_acquire_write(&(thread->lock));
	string_t* old_name=thread->name;
	thread->name=smm_alloc(buffer,0);
	smm_dealloc(old_name);
	rwlock_release_write(&(thread->lock));
	handle_release(handle);
	return ERROR_OK;
}
