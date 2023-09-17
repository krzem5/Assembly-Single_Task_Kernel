#include <kernel/acpi/fadt.h>
#include <kernel/apic/lapic.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/ap_startup.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/fpu/fpu.h>
#include <kernel/gdt/gdt.h>
#include <kernel/idt/idt.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "cpu"



cpu_data_t* cpu_data;
u16 cpu_count;
u8 cpu_bsp_core_id;



void _cpu_init_core(void){
	u8 index=msr_get_apic_id();
	LOG("Initializing core #%u (%u:%u:%u:%u)...",index,(cpu_data+index)->topology.domain,(cpu_data+index)->topology.chip,(cpu_data+index)->topology.core,(cpu_data+index)->topology.thread);
	INFO("Loading IDT, GDT, TSS, FS and GS...");
	idt_enable();
	gdt_enable(&((cpu_data+index)->tss));
	msr_set_fs_base(NULL);
	msr_set_gs_base(cpu_data+index,0);
	msr_set_gs_base(cpu_data+index,1);
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
	topology_compute(index,&((cpu_data+index)->topology));
	CPU_DATA->flags|=CPU_FLAG_ONLINE;
	if (index!=cpu_bsp_core_id){
		scheduler_start();
	}
}



void cpu_init(u16 count){
	LOG("Initializing CPU manager...");
	INFO("CPU count: %u",count);
	cpu_count=count;
	cpu_data=umm_alloc(count*sizeof(cpu_data_t));
	for (u16 i=0;i<count;i++){
		(cpu_data+i)->index=i;
		(cpu_data+i)->flags=0;
		(cpu_data+i)->kernel_rsp=((u64)((cpu_data+i)->scheduler_stack))+CPU_SCHEDULER_STACK_SIZE;
		(cpu_data+i)->kernel_cr3=vmm_kernel_pagemap.toplevel;
		(cpu_data+i)->tss.rsp0=((u64)((cpu_data+i)->interrupt_stack))+CPU_INTERRUPT_STACK_SIZE;
		(cpu_data+i)->tss.ist1=0;
		(cpu_data+i)->tss.ist2=(cpu_data+i)->kernel_rsp;
		(cpu_data+i)->scheduler=scheduler_new();
	}
	cpu_bsp_core_id=msr_get_apic_id();
	INFO("BSP APIC id: #%u",cpu_bsp_core_id);
}



void cpu_register_core(u8 apic_id){
	LOG("Registering CPU core #%u",apic_id);
	(cpu_data+apic_id)->flags|=CPU_FLAG_PRESENT;
}



void cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	vmm_map_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS,CPU_AP_STARTUP_MEMORY_ADDRESS,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	cpu_ap_startup_init();
	for (u16 i=0;i<cpu_count;i++){
		if (!((cpu_data+i)->flags&CPU_FLAG_PRESENT)){
			ERROR("Unused CPU core: #%u",i);
			panic("Unused CPU core",0);
		}
		if (i==cpu_bsp_core_id){
			continue;
		}
		cpu_ap_startup_set_stack_top((cpu_data+i)->kernel_rsp);
		lapic_send_ipi(i,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_LEVEL_ASSERT|APIC_ICR0_DELIVERY_MODE_INIT);
		lapic_send_ipi(i,APIC_ICR0_TRIGGER_MODE_LEVEL|APIC_ICR0_DELIVERY_MODE_INIT);
		COUNTER_SPINLOOP(0xfff);
		for (u8 j=0;j<2;j++){
			lapic_send_ipi(i,APIC_ICR0_DELIVERY_MODE_STARTUP|(CPU_AP_STARTUP_MEMORY_ADDRESS>>PAGE_SIZE_SHIFT));
		}
		const volatile u8* flags=&((cpu_data+i)->flags);
		SPINLOOP(!((*flags)&CPU_FLAG_ONLINE));
	}
	vmm_unmap_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS);
	_cpu_init_core();
}



void cpu_core_start(u8 index,u64 start_address,u64 arg1,u64 arg2){
	ERROR("Unimplemented: cpu_core_start");
}



void cpu_core_stop(void){
	scheduler_dequeue();
}
