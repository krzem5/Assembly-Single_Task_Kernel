#include <kernel/apic/lapic.h>
#include <kernel/cpu/ap_startup.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/error/error.h>
#include <kernel/fpu/fpu.h>
#include <kernel/gdt/gdt.h>
#include <kernel/idt/idt.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>
#include <kernel/util/spinloop.h>
#define KERNEL_LOG_NAME "cpu"



static KERNEL_ATOMIC u16 KERNEL_EARLY_WRITE _cpu_online_count=0;
static u16 KERNEL_EARLY_WRITE _cpu_bootstrap_core_apic_id;
static cpu_header_t KERNEL_EARLY_WRITE _cpu_early_header={
	.index=0,
	.current_thread=NULL
};

KERNEL_PUBLIC u16 KERNEL_INIT_WRITE cpu_count;
CPU_LOCAL_DATA(cpu_extra_data_t,cpu_extra_data);



void _cpu_init_core(void);



static void KERNEL_EARLY_EXEC _wakeup_cpu(u8 index){
	if (index>=cpu_count){
		return;
	}
	if (index==_cpu_bootstrap_core_apic_id){
		_cpu_init_core();
		return;
	}
	lapic_send_ipi(index,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_LEVEL_ASSERT|APIC_ICR0_DELIVERY_MODE_INIT);
	lapic_send_ipi(index,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_DELIVERY_MODE_INIT);
	COUNTER_SPINLOOP(0xfff);
	for (u8 i=0;i<2;i++){
		lapic_send_ipi(index,APIC_ICR0_DELIVERY_MODE_STARTUP|(CPU_AP_STARTUP_MEMORY_ADDRESS>>PAGE_SIZE_SHIFT));
	}
}



void KERNEL_EARLY_EXEC _cpu_init_core(void){
	u8 index=msr_get_apic_id();
	idt_enable();
	gdt_enable(&((cpu_extra_data+index)->tss));
	msr_set_fs_base(0);
	msr_set_gs_base((u64)(cpu_extra_data+index),0);
	msr_set_gs_base((u64)(cpu_extra_data+index),1);
	topology_compute(index,&((cpu_extra_data+index)->topology));
	LOG("Initializing core #%u (%u:%u:%u:%u)...",index,(cpu_extra_data+index)->topology.domain,(cpu_extra_data+index)->topology.chip,(cpu_extra_data+index)->topology.core,(cpu_extra_data+index)->topology.thread);
	INFO("Enabling FPU...");
	fpu_enable();
	INFO("Enabling SYSCALL/SYSRET...");
	syscall_enable();
	INFO("Enabling RDTSC...");
	msr_enable_rdtsc();
	INFO("Enabling lAPIC...");
	lapic_enable();
	_cpu_online_count++;
	LOG("Core #%u initialized",index);
	_wakeup_cpu((index<<1)+1);
	_wakeup_cpu((index+1)<<1);
	if (index!=_cpu_bootstrap_core_apic_id){
		scheduler_yield();
	}
}



void KERNEL_EARLY_EXEC cpu_init_early_header(void){
	msr_set_gs_base((u64)(&_cpu_early_header),0);
	msr_set_gs_base((u64)(&_cpu_early_header),1);
}



void KERNEL_EARLY_EXEC cpu_init(u16 count){
	LOG("Initializing CPU manager...");
	INFO("CPU count: %u",count);
	cpu_count=count;
	cpu_local_init();
	pmm_counter_descriptor_t* pmm_counter=pmm_alloc_counter("kernel.cpu.stack");
	for (u16 i=0;i<count;i++){
		(cpu_extra_data+i)->header.index=i;
		(cpu_extra_data+i)->header.kernel_rsp=pmm_alloc(CPU_SCHEDULER_STACK_PAGE_COUNT,pmm_counter,0)+(CPU_SCHEDULER_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT)+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		(cpu_extra_data+i)->tss.rsp0=pmm_alloc(CPU_INTERRUPT_STACK_PAGE_COUNT,pmm_counter,0)+(CPU_INTERRUPT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT)+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		(cpu_extra_data+i)->tss.ist1=pmm_alloc(CPU_PAGE_FAULT_STACK_PAGE_COUNT,pmm_counter,0)+(CPU_PAGE_FAULT_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT)+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		(cpu_extra_data+i)->tss.ist2=(cpu_extra_data+i)->header.kernel_rsp;
	}
	_cpu_bootstrap_core_apic_id=msr_get_apic_id();
	INFO("Bootstrap CPU APIC id: #%u",_cpu_bootstrap_core_apic_id);
}



void KERNEL_EARLY_EXEC cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	pmm_counter_descriptor_t* cpu_pmm_counter=pmm_alloc_counter("kernel.cpu");
	u64* cpu_stack_list=(u64*)(pmm_alloc(pmm_align_up_address(cpu_count*sizeof(u64))>>PAGE_SIZE_SHIFT,cpu_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u16 i=0;i<cpu_count;i++){
		cpu_stack_list[i]=(cpu_extra_data+i)->header.kernel_rsp;
	}
	vmm_map_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS,CPU_AP_STARTUP_MEMORY_ADDRESS,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	cpu_ap_startup_init(cpu_stack_list);
	_cpu_online_count=0;
	_wakeup_cpu(0);
	SPINLOOP(_cpu_online_count!=cpu_count);
	vmm_unmap_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS);
	pmm_dealloc(((u64)cpu_stack_list)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(cpu_count*sizeof(u64))>>PAGE_SIZE_SHIFT,cpu_pmm_counter);
}



error_t syscall_cpu_get_count(void){
	return cpu_count;
}
