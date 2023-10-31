#include <kernel/apic/lapic.h>
#include <kernel/cpu/ap_startup.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
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
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "cpu"



static pmm_counter_descriptor_t _cpu_pmm_counter=PMM_COUNTER_INIT_STRUCT("cpu");



static _Atomic u16 KERNEL_INIT_WRITE _cpu_online_count;
static u16 KERNEL_INIT_WRITE _cpu_bootstra_core_apic_id;

u16 KERNEL_INIT_WRITE cpu_count;
KERNEL_INIT_WRITE CPU_LOCAL_DATA(cpu_extra_data_t,cpu_extra_data);



void _cpu_init_core(void);



static void _wakeup_cpu(u8 idx){
	if (idx>=cpu_count){
		return;
	}
	if (idx==_cpu_bootstra_core_apic_id){
		_cpu_init_core();
		return;
	}
	lapic_send_ipi(idx,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_LEVEL_ASSERT|APIC_ICR0_DELIVERY_MODE_INIT);
	lapic_send_ipi(idx,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_DELIVERY_MODE_INIT);
	COUNTER_SPINLOOP(0xfff);
	for (u8 i=0;i<2;i++){
		lapic_send_ipi(idx,APIC_ICR0_DELIVERY_MODE_STARTUP|(CPU_AP_STARTUP_MEMORY_ADDRESS>>PAGE_SIZE_SHIFT));
	}
}



void _cpu_init_core(void){
	u8 index=msr_get_apic_id();
	LOG("Initializing core #%u (%u:%u:%u:%u)...",index,(cpu_extra_data+index)->topology.domain,(cpu_extra_data+index)->topology.chip,(cpu_extra_data+index)->topology.core,(cpu_extra_data+index)->topology.thread);
	INFO("Loading IDT, GDT, TSS, FS and GS...");
	idt_enable();
	gdt_enable(&((cpu_extra_data+index)->tss));
	msr_set_fs_base(0);
	msr_set_gs_base((u64)(cpu_extra_data+index),0);
	msr_set_gs_base((u64)(cpu_extra_data+index),1);
	INFO("Enabling FPU...");
	fpu_enable();
	INFO("Enabling SYSCALL/SYSRET...");
	syscall_enable();
	INFO("Enabling RDTSC...");
	msr_enable_rdtsc();
	INFO("Enabling lAPIC...");
	lapic_enable();
	INFO("Calcularing topology...");
	topology_compute(index,&((cpu_extra_data+index)->topology));
	_cpu_online_count++;
	_wakeup_cpu((index<<1)+1);
	_wakeup_cpu((index+1)<<1);
	if (index!=_cpu_bootstra_core_apic_id){
		scheduler_start();
	}
}



void cpu_init(u16 count){
	LOG("Initializing CPU manager...");
	INFO("CPU count: %u",count);
	cpu_count=count;
	cpu_local_init();
	for (u16 i=0;i<count;i++){
		(cpu_extra_data+i)->header.index=i;
		(cpu_extra_data+i)->header.kernel_rsp=((u64)((cpu_extra_data+i)->scheduler_stack))+CPU_SCHEDULER_STACK_SIZE;
		(cpu_extra_data+i)->tss.rsp0=((u64)((cpu_extra_data+i)->interrupt_stack))+CPU_INTERRUPT_STACK_SIZE;
		(cpu_extra_data+i)->tss.ist2=(cpu_extra_data+i)->header.kernel_rsp;
	}
	_cpu_bootstra_core_apic_id=msr_get_apic_id();
	INFO("Bootstrap CPU APIC id: #%u",_cpu_bootstra_core_apic_id);
}



void cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	u64* cpu_stack_list=(u64*)(pmm_alloc(pmm_align_up_address(cpu_count*sizeof(u64))>>PAGE_SIZE_SHIFT,&_cpu_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u16 i=0;i<cpu_count;i++){
		cpu_stack_list[i]=(cpu_extra_data+i)->header.kernel_rsp;
	}
	vmm_map_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS,CPU_AP_STARTUP_MEMORY_ADDRESS,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	cpu_ap_startup_init(cpu_stack_list);
	_cpu_online_count=0;
	_wakeup_cpu(0);
	SPINLOOP(_cpu_online_count!=cpu_count);
	vmm_unmap_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS);
	pmm_dealloc(((u64)cpu_stack_list)-VMM_HIGHER_HALF_ADDRESS_OFFSET,pmm_align_up_address(cpu_count*sizeof(u64))>>PAGE_SIZE_SHIFT,&_cpu_pmm_counter);
}
