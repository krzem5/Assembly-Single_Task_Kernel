#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/isr/isr.h>
#include <kernel/isr/pf.h>
#include <kernel/kernel.h>
#include <kernel/lock/rwlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/shutdown/shutdown.h>
#include <kernel/symbol/symbol.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "isr"



#ifdef KERNEL_RELEASE
#define USER_STACKTRACE 0
#else
#define USER_STACKTRACE 1
#endif



static rwlock_t _isr_next_irq_lock=RWLOCK_INIT_STRUCT;
static u8 _isr_next_irq_index=33;
static CPU_LOCAL_DATA(u32,_isr_nested_interrupt);

KERNEL_PUBLIC KERNEL_ATOMIC irq_handler_t irq_handlers[223];
KERNEL_PUBLIC void* KERNEL_ATOMIC irq_handler_contexts[223];



KERNEL_EARLY_INIT(){
	LOG("Initializing IRQ handlers...");
	for (u8 i=0;i<223;i++){
		irq_handlers[i]=NULL;
		irq_handler_contexts[i]=NULL;
	}
	rwlock_init(&_isr_next_irq_lock);
}



KERNEL_PUBLIC u8 isr_allocate(void){
	rwlock_acquire_write(&_isr_next_irq_lock);
	if (_isr_next_irq_index>=0xfe){
		panic("Not enough IRQs");
	}
	u8 out=_isr_next_irq_index;
	_isr_next_irq_index++;
	rwlock_release_write(&_isr_next_irq_lock);
	return out;
}



void _isr_handler(isr_state_t* isr_state){
	if (isr_state->isr==32){
		scheduler_isr_handler(isr_state);
		return;
	}
	scheduler_set_irq_context(1);
	scheduler_pause();
	u32 mask=1<<(isr_state->isr==14);
	if ((*CPU_LOCAL(_isr_nested_interrupt))&mask){
		ERROR("Nested %s",(isr_state->isr==14?"page fault":"interrupt"));
		goto _crash;
	}
	*CPU_LOCAL(_isr_nested_interrupt)|=mask;
	if (isr_state->isr==14&&pf_handle_fault(isr_state)){
		*CPU_LOCAL(_isr_nested_interrupt)&=~mask;
		scheduler_resume(0);
		scheduler_set_irq_context(0);
		return;
	}
	if (isr_state->isr>32){
		lapic_eoi();
		irq_handler_t handler=IRQ_HANDLER(isr_state->isr);
		if (handler){
			handler(IRQ_HANDLER_CTX(isr_state->isr));
		}
		*CPU_LOCAL(_isr_nested_interrupt)&=~mask;
		scheduler_resume(0);
		scheduler_set_irq_context(0);
		return;
	}
_crash:
	if (isr_state->isr==14){
		ERROR("Page fault");
		ERROR("Address: %p, Error: %p",pf_get_fault_address(),isr_state->error);
	}
	else{
		ERROR("Crash");
	}
	WARN("ISR %u:",isr_state->isr);
	WARN("cpu    = %p",CPU_HEADER_DATA->index);
	WARN("es     = %p",isr_state->es);
	WARN("ds     = %p",isr_state->ds);
	WARN("rax    = %p",isr_state->rax);
	WARN("rbx    = %p",isr_state->rbx);
	WARN("rcx    = %p",isr_state->rcx);
	WARN("rdx    = %p",isr_state->rdx);
	WARN("rsi    = %p",isr_state->rsi);
	WARN("rdi    = %p",isr_state->rdi);
	WARN("rbp    = %p",isr_state->rbp);
	WARN("r8     = %p",isr_state->r8);
	WARN("r9     = %p",isr_state->r9);
	WARN("r10    = %p",isr_state->r10);
	WARN("r11    = %p",isr_state->r11);
	WARN("r12    = %p",isr_state->r12);
	WARN("r13    = %p",isr_state->r13);
	WARN("r14    = %p",isr_state->r14);
	WARN("r15    = %p",isr_state->r15);
	WARN("error  = %p",isr_state->error);
	WARN("rip    = %p",isr_state->rip);
	WARN("cs     = %p",isr_state->cs);
	WARN("rflags = %p",isr_state->rflags);
	WARN("rsp    = %p",isr_state->rsp);
	WARN("ss     = %p",isr_state->ss);
	if (USER_STACKTRACE||isr_state->cs==0x08){
		vmm_pagemap_t current_pagemap;
		vmm_get_pagemap(&current_pagemap);
		u64 rip=isr_state->rip;
		u64 rbp=isr_state->rbp;
		while (1){
			const symbol_t* symbol=symbol_lookup(rip);
			if (symbol){
				WARN("[%u] %s:%s+%u",CPU_HEADER_DATA->index,symbol->module,symbol->name->data,rip-symbol->rb_node.key);
			}
			else{
				WARN("[%u] %p",CPU_HEADER_DATA->index,rip);
			}
			if (!rbp){
				break;
			}
			if (!vmm_virtual_to_physical(&current_pagemap,rbp)||!vmm_virtual_to_physical(&current_pagemap,rbp+8)){
				WARN("[%u] <rbp: %p>",CPU_HEADER_DATA->index,rbp);
				break;
			}
			rip=*((u64*)(rbp+8));
			rbp=*((u64*)rbp);
		}
	}
	*CPU_LOCAL(_isr_nested_interrupt)&=~mask;
	scheduler_set_irq_context(0);
	if (CPU_HEADER_DATA->current_thread){
		thread_terminate(NULL);
	}
	shutdown(SHUTDOWN_FLAG_NO_CLEANUP);
	for (;;);
}
