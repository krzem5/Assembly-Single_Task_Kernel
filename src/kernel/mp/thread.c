#include <kernel/cpu/cpu.h>
#include <kernel/fpu/fpu.h>
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/mp/thread_list.h>
#include <kernel/scheduler/cpu_mask.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "thread"



#define SET_KERNEL_THREAD_ARG(register) \
	if ((arg_count)){ \
		(arg_count)--; \
		out->gpr_state.register=__builtin_va_arg(va,u64); \
	}



static pmm_counter_descriptor_t* _thread_user_stack_pmm_counter=NULL;
static pmm_counter_descriptor_t* _thread_kernel_stack_pmm_counter=NULL;
static pmm_counter_descriptor_t* _thread_pf_stack_pmm_counter=NULL;
static omm_allocator_t* _thread_allocator=NULL;
static omm_allocator_t* _thread_fpu_state_allocator=NULL;

KERNEL_PUBLIC handle_type_t thread_handle_type=0;



static void _thread_handle_destructor(handle_t* handle){
	thread_t* thread=handle->object;
	if (thread->state.type!=THREAD_STATE_TYPE_TERMINATED){
		panic("Unterminated thread not referenced");
	}
	cpu_mask_delete(thread->cpu_mask);
	process_t* process=thread->process;
	if (thread_list_remove(&(process->thread_list),thread)){
		handle_release(&(process->handle));
	}
	omm_dealloc(_thread_allocator,thread);
}



static thread_t* _thread_alloc(process_t* process,u64 user_stack_size,u64 kernel_stack_size){
	if (!_thread_user_stack_pmm_counter){
		_thread_user_stack_pmm_counter=pmm_alloc_counter("user_stack");
	}
	if (!_thread_kernel_stack_pmm_counter){
		_thread_kernel_stack_pmm_counter=pmm_alloc_counter("kernel_stack");
	}
	if (!_thread_pf_stack_pmm_counter){
		_thread_pf_stack_pmm_counter=pmm_alloc_counter("pf_stack");
	}
	if (!_thread_allocator){
		_thread_allocator=omm_init("thread",sizeof(thread_t),8,4,pmm_alloc_counter("omm_thread"));
		spinlock_init(&(_thread_allocator->lock));
	}
	if (!_thread_fpu_state_allocator){
		_thread_fpu_state_allocator=omm_init("fpu_state",fpu_state_size,64,4,pmm_alloc_counter("omm_thread_fpu_state"));
		spinlock_init(&(_thread_fpu_state_allocator->lock));
	}
	if (!thread_handle_type){
		thread_handle_type=handle_alloc("thread",_thread_handle_destructor);
	}
	user_stack_size=pmm_align_up_address(user_stack_size);
	kernel_stack_size=pmm_align_up_address(kernel_stack_size);
	thread_t* out=omm_alloc(_thread_allocator);
	memset(out,0,sizeof(thread_t));
	out->header.current_thread=out;
	handle_new(out,thread_handle_type,&(out->handle));
	spinlock_init(&(out->lock));
	out->process=process;
	if (user_stack_size){
		out->user_stack_region=mmap_alloc(&(process->mmap),0,user_stack_size,_thread_user_stack_pmm_counter,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_USER|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
		if (!out->user_stack_region){
			panic("Unable to reserve thread stack");
		}
	}
	else{
		out->user_stack_region=NULL;
	}
	out->kernel_stack_region=mmap_alloc(&(process_kernel->mmap),0,kernel_stack_size,_thread_kernel_stack_pmm_counter,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
	if (!out->kernel_stack_region){
		panic("Unable to reserve thread stack");
	}
	out->pf_stack_region=mmap_alloc(&(process_kernel->mmap),0,CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT,_thread_pf_stack_pmm_counter,MMAP_REGION_FLAG_COMMIT|MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE,NULL);
	if (!out->pf_stack_region){
		panic("Unable to reserve thread stack");
	}
	out->fpu_state=omm_alloc(_thread_fpu_state_allocator);
	fpu_init(out->fpu_state);
	out->cpu_mask=cpu_mask_new();
	out->priority=SCHEDULER_PRIORITY_NORMAL;
	out->state_not_present=0;
	out->state.type=THREAD_STATE_TYPE_NONE;
	out->signal_state=NULL;
	thread_list_add(&(process->thread_list),out);
	return out;
}



KERNEL_PUBLIC thread_t* thread_new_user_thread(process_t* process,u64 rip,u64 stack_size){
	thread_t* out=_thread_alloc(process,stack_size,CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	out->header.kernel_rsp=out->kernel_stack_region->rb_node.key+(CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
	out->gpr_state.rip=rip;
	out->gpr_state.rsp=out->user_stack_region->rb_node.key+stack_size;
	out->gpr_state.cs=0x23;
	out->gpr_state.ds=0x1b;
	out->gpr_state.es=0x1b;
	out->gpr_state.ss=0x1b;
	out->gpr_state.rflags=0x0000000202;
	out->fs_gs_state.fs=0;
	out->fs_gs_state.gs=0;
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC thread_t* thread_new_kernel_thread(process_t* process,void* func,u64 stack_size,u8 arg_count,...){
	if (arg_count>6){
		panic("Too many kernel thread arguments");
	}
	_Bool start_thread=!process;
	thread_t* out=_thread_alloc((process?process:process_kernel),0,stack_size);
	out->gpr_state.rip=(u64)_thread_bootstrap_kernel_thread;
	out->gpr_state.rax=(u64)func;
	out->gpr_state.rsp=out->kernel_stack_region->rb_node.key+stack_size;
	__builtin_va_list va;
	__builtin_va_start(va,arg_count);
	SET_KERNEL_THREAD_ARG(rdi);
	SET_KERNEL_THREAD_ARG(rsi);
	SET_KERNEL_THREAD_ARG(rdx);
	SET_KERNEL_THREAD_ARG(rcx);
	SET_KERNEL_THREAD_ARG(r8);
	SET_KERNEL_THREAD_ARG(r9);
	__builtin_va_end(va);
	out->gpr_state.cs=0x08;
	out->gpr_state.ds=0x10;
	out->gpr_state.es=0x10;
	out->gpr_state.ss=0x10;
	out->gpr_state.rflags=0x0000000202;
	out->fs_gs_state.fs=0;
	out->fs_gs_state.gs=(u64)out;
	handle_finish_setup(&(out->handle));
	if (start_thread){
		scheduler_enqueue_thread(out);
	}
	return out;
}



KERNEL_PUBLIC void thread_delete(thread_t* thread){
	spinlock_acquire_exclusive(&(thread->lock));
	process_t* process=thread->process;
	if (thread->user_stack_region){
		mmap_dealloc_region(&(process->mmap),thread->user_stack_region);
	}
	mmap_dealloc_region(&(process_kernel->mmap),thread->kernel_stack_region);
	mmap_dealloc_region(&(process_kernel->mmap),thread->pf_stack_region);
	omm_dealloc(_thread_fpu_state_allocator,thread->fpu_state);
	if (handle_release(&(thread->handle))){
		spinlock_release_exclusive(&(thread->lock));
	}
}



KERNEL_PUBLIC void KERNEL_NORETURN thread_terminate(void){
	scheduler_pause();
	thread_t* thread=CPU_HEADER_DATA->current_thread;
	spinlock_acquire_exclusive(&(thread->lock));
	thread->state.type=THREAD_STATE_TYPE_TERMINATED;
	spinlock_release_exclusive(&(thread->lock));
	scheduler_yield();
	for (;;);
}



KERNEL_PUBLIC void thread_await_event(event_t* event){
	scheduler_pause();
	thread_t* thread=CPU_HEADER_DATA->current_thread;
	spinlock_acquire_exclusive(&(event->lock));
	spinlock_acquire_exclusive(&(thread->lock));
	thread->state.type=THREAD_STATE_TYPE_AWAITING_EVENT;
	thread->state.event.event=event;
	thread->state.event.next=NULL;
	if (!event->head){
		event->head=thread;
		event->tail=thread;
	}
	else{
		spinlock_acquire_exclusive(&(event->tail->lock));
		event->tail->state.event.next=thread;
		spinlock_release_exclusive(&(event->tail->lock));
		event->tail=thread;
	}
	thread->state_not_present=1;
	spinlock_release_exclusive(&(thread->lock));
	spinlock_release_exclusive(&(event->lock));
	scheduler_yield();
}
