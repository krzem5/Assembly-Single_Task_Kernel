#include <kernel/apic/lapic.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/isr/isr.h>
#include <kernel/isr/pf.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/event.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/signal/signal.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "isr"



typedef struct _ISR_SIGNAL_CONTEXT{
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 error;
	u64 rip;
	u64 rflags;
	u64 rsp;
} isr_signal_context_t;



static const signal_type_t _isr_to_signal_type[]={
	[0]=SIGNAL_TYPE_ZDE,
	[6]=SIGNAL_TYPE_IOE,
	[13]=SIGNAL_TYPE_GPF,
	[14]=SIGNAL_TYPE_PF,
	[16]=SIGNAL_TYPE_FPE,
	[19]=SIGNAL_TYPE_FPE
};

static u8 _isr_next_irq_index=33;

event_t* KERNEL_INIT_WRITE irq_events[223];



void isr_init(void){
	LOG("Initializing ISR events...");
	for (u8 i=0;i<223;i++){
		irq_events[i]=event_new();
	}
}



u8 isr_allocate(void){
	if (_isr_next_irq_index>=0xfe){
		panic("Not enough IRQs");
	}
	u8 out=_isr_next_irq_index;
	_isr_next_irq_index++;
	return out;
}



void _isr_handler(isr_state_t* isr_state){
	if (isr_state->isr==32){
		scheduler_isr_handler(isr_state);
		return;
	}
	if (isr_state->isr==14&&pf_handle_fault(isr_state)){
		return;
	}
	if (isr_state->isr>32){
		lapic_eoi();
		event_dispatch(IRQ_EVENT(isr_state->isr),1);
		return;
	}
	if (CPU_HEADER_DATA->current_thread&&((isr_state->rflags>>12)&3)==3&&(0b00000000000010010110000001000001&(1<<isr_state->isr))){
		isr_signal_context_t ctx={
			isr_state->rax,
			isr_state->rbx,
			isr_state->rcx,
			isr_state->rdx,
			isr_state->rsi,
			isr_state->rdi,
			isr_state->rbp,
			isr_state->r8,
			isr_state->r9,
			isr_state->r10,
			isr_state->r11,
			isr_state->r12,
			isr_state->r13,
			isr_state->r14,
			isr_state->r15,
			isr_state->error,
			isr_state->rip,
			isr_state->rflags,
			isr_state->rsp
		};
		signal_send(CPU_HEADER_DATA->current_thread,isr_state,_isr_to_signal_type[isr_state->isr],&ctx,sizeof(isr_signal_context_t));
		return;
	}
	if (isr_state->isr==8&&!CPU_LOCAL(cpu_extra_data)->tss.ist1){
		panic("Page fault stack not present");
	}
	else if (isr_state->isr==14){
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
	if (isr_state->cs==8){
		u64 offset;
		const char* func_name=kernel_lookup_symbol(isr_state->rip,&offset);
		if (func_name){
			LOG("[%u] %s+%u",CPU_HEADER_DATA->index,func_name,offset);
		}
		else{
			LOG("[%u] %p",CPU_HEADER_DATA->index,isr_state->rip);
		}
		u64 rbp=isr_state->rbp;
		vmm_pagemap_t current_pagemap;
		vmm_get_pagemap(&current_pagemap);
		while (rbp){
			if (vmm_virtual_to_physical(&current_pagemap,rbp)){
				func_name=kernel_lookup_symbol(*((u64*)(rbp+8)),&offset);
			}
			else{
				LOG("[%u] ??? [%p]",CPU_HEADER_DATA->index,rbp);
				break;
			}
			if (func_name){
				LOG("[%u] %s+%u",CPU_HEADER_DATA->index,func_name,offset);
			}
			else{
				LOG("[%u] %p",CPU_HEADER_DATA->index,*((u64*)(rbp+8)));
			}
			rbp=*((u64*)rbp);
		}
	}
	for (;;);
}
