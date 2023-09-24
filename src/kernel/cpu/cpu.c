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



static volatile _Bool _cpu_online;

CPU_LOCAL_DATA(cpu_extra_data_t,cpu_extra_data);
u16 cpu_count;
u8 cpu_bsp_core_id;



static void _wakeup_cpu(u8 idx){
	if (idx==cpu_bsp_core_id){
		return;
	}
	_cpu_online=0;
	cpu_ap_startup_set_stack_top((cpu_extra_data+idx)->header.kernel_rsp);
	lapic_send_ipi(idx,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_LEVEL_ASSERT|APIC_ICR0_DELIVERY_MODE_INIT);
	lapic_send_ipi(idx,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_DELIVERY_MODE_INIT);
	COUNTER_SPINLOOP(0xfff);
	for (u8 i=0;i<2;i++){
		lapic_send_ipi(idx,APIC_ICR0_DELIVERY_MODE_STARTUP|(CPU_AP_STARTUP_MEMORY_ADDRESS>>PAGE_SIZE_SHIFT));
	}
	SPINLOOP(!_cpu_online);
}



void _cpu_init_core(void){
	u8 index=msr_get_apic_id();
	LOG("Initializing core #%u (%u:%u:%u:%u)...",index,(cpu_extra_data+index)->topology.domain,(cpu_extra_data+index)->topology.chip,(cpu_extra_data+index)->topology.core,(cpu_extra_data+index)->topology.thread);
	INFO("Loading IDT, GDT, TSS, FS and GS...");
	idt_enable();
	gdt_enable(&((cpu_extra_data+index)->tss));
	msr_set_fs_base(NULL);
	msr_set_gs_base(cpu_extra_data+index,0);
	msr_set_gs_base(cpu_extra_data+index,1);
	INFO("Enabling FPU...");
	fpu_enable();
	INFO("Enabling SYSCALL/SYSRET...");
	syscall_enable();
	INFO("Enabling RDTSC...");
	msr_enable_rdtsc();
	INFO("Enabling FSGSBASE...");
	msr_enable_fsgsbase();
	INFO("Enabling lAPIC...");
	lapic_enable();
	INFO("Calcularing topology...");
	topology_compute(index,&((cpu_extra_data+index)->topology));
	_cpu_online=1;
	if (index!=cpu_bsp_core_id){
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
	cpu_bsp_core_id=msr_get_apic_id();
	INFO("BSP APIC id: #%u",cpu_bsp_core_id);
	msr_set_gs_base(cpu_extra_data+cpu_bsp_core_id,0);
}



void cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	vmm_map_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS,CPU_AP_STARTUP_MEMORY_ADDRESS,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	cpu_ap_startup_init();
	for (u16 i=0;i<cpu_count;i++){
		_wakeup_cpu(i);
	}
	vmm_unmap_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS);
	_cpu_init_core();
}
