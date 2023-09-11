#include <kernel/acpi/fadt.h>
#include <kernel/apic/lapic.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/ap_startup.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
#include <kernel/gdt/gdt.h>
#include <kernel/idt/idt.h>
#include <kernel/log/log.h>
#include <kernel/memory/kmm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/syscall/syscall.h>
#include <kernel/topology/topology.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "cpu"



#define LOADER_FILE_PATH "/kernel/loader.elf"




cpu_data_t* cpu_data;
u16 cpu_count;
u8 cpu_bsp_core_id;



void KERNEL_NORETURN _cpu_init_core(void){
	u8 index=msr_get_apic_id();
	LOG("Initializing core #%u (%u:%u:%u:%u)...",index,(cpu_data+index)->topology.domain,(cpu_data+index)->topology.chip,(cpu_data+index)->topology.core,(cpu_data+index)->topology.thread);
	INFO("Loading IDT, GDT, TSS, FS and GS...");
	idt_enable();
	gdt_enable(&((cpu_data+index)->tss));
	msr_set_fs_base(NULL);
	msr_set_gs_base(cpu_data+index,0);
	msr_set_gs_base(cpu_data+index,1);
	INFO("Enabling SIMD...");
	msr_enable_simd();
	INFO("Enabling SYSCALL/SYSRET...");
	syscall_enable();
	INFO("Enabling RDTSC...");
	msr_enable_rdtsc();
	INFO("Enabling FSGSBASE...");
	msr_enable_fsgsbase();
	INFO("Enabling lAPIC...");
	lapic_enable();
	CPU_DATA->flags|=CPU_FLAG_ONLINE;
	syscall_jump_to_user_mode();
}



void cpu_init(u16 count){
	LOG("Initializing CPU manager...");
	INFO("CPU count: %u",count);
	cpu_count=count;
	cpu_data=umm_alloc(count*sizeof(cpu_data_t));
	u64 user_stacks=pmm_alloc(cpu_count*CPU_USER_STACK_PAGE_COUNT,PMM_COUNTER_USER_STACK);
	u64 kernel_stacks=pmm_alloc(cpu_count*CPU_KERNEL_STACK_PAGE_COUNT,PMM_COUNTER_KERNEL_STACK);
	for (u16 i=0;i<count;i++){
		kernel_stacks+=CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT;
		(cpu_data+i)->index=i;
		(cpu_data+i)->flags=0;
		(cpu_data+i)->kernel_rsp=kernel_stacks;
		(cpu_data+i)->isr_rsp=(u64)((cpu_data+i)->isr_stack+ISR_STACK_SIZE);
		(cpu_data+i)->user_func=0;
		(cpu_data+i)->user_func_arg[0]=0;
		(cpu_data+i)->user_func_arg[1]=0;
		(cpu_data+i)->user_rsp_top=UMM_STACK_TOP-i*(CPU_USER_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
		(cpu_data+i)->tss.rsp0=(cpu_data+i)->isr_rsp;
	}
	cpu_bsp_core_id=msr_get_apic_id();
	INFO("BSP APIC id: #%u",cpu_bsp_core_id);
	umm_set_user_stacks(user_stacks,cpu_count*CPU_USER_STACK_PAGE_COUNT);
}



void cpu_register_core(u8 apic_id){
	LOG("Registering CPU core #%u",apic_id);
	(cpu_data+apic_id)->flags|=CPU_FLAG_PRESENT;
	topology_compute(apic_id,&((cpu_data+apic_id)->topology));
}



void cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	cpu_ap_startup_init();
	for (u16 i=0;i<cpu_count;i++){
		if (!((cpu_data+i)->flags&CPU_FLAG_PRESENT)){
			ERROR("Unused CPU core: #%u",i);
			for (;;);
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
	(cpu_data+cpu_bsp_core_id)->user_func_arg[0]=0;
	(cpu_data+cpu_bsp_core_id)->user_func_arg[1]=0;
	(cpu_data+cpu_bsp_core_id)->user_func=elf_load(LOADER_FILE_PATH);
	_cpu_init_core();
}



void cpu_core_start(u8 index,u64 start_address,u64 arg1,u64 arg2){
	(((volatile cpu_data_t*)cpu_data)+index)->user_func_arg[0]=arg1;
	(((volatile cpu_data_t*)cpu_data)+index)->user_func_arg[1]=arg2;
	(((volatile cpu_data_t*)cpu_data)+index)->user_func=start_address;
	if (CPU_DATA->index==index){
		syscall_jump_to_user_mode();
	}
	lapic_send_ipi(index,LAPIC_WAKEUP_VECTOR);
}



void KERNEL_NORETURN cpu_core_stop(void){
	if (CPU_DATA->index==cpu_bsp_core_id){
		CPU_DATA->user_func_arg[0]=0;
		CPU_DATA->user_func_arg[1]=0;
		CPU_DATA->user_func=elf_load(LOADER_FILE_PATH);
	}
	syscall_jump_to_user_mode();
}
