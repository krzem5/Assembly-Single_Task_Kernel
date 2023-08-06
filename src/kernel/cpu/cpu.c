#include <kernel/acpi/fadt.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/ap_startup.h>
#include <kernel/cpu/cpu.h>
#include <kernel/gdt/gdt.h>
#include <kernel/idt/idt.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "cpu"



#define APIC_DELIVERY_MODE_INIT 0x0500
#define APIC_DELIVERY_MODE_STARTUP 0x0600
#define APIC_DELIVERY_STATUS 0x1000
#define APIC_TRIGGER_MODE_LEVEL 0x8000
#define APIC_INTR_COMMAND_1_ASSERT 0x4000

#define CPU_FLAG_PRESENT 1
#define CPU_FLAG_ONLINE 2

#define ISR_STACK_SIZE 56

#define CPU_DATA ((volatile __seg_gs cpu_data_t*)NULL)



typedef struct _CPU_DATA{
	u8 index;
	u8 flags;
	u8 _padding[5];
	u64 stack_top;
	u64 user_rsp;
	u64 isr_stack_top;
	u64 user_func;
	u64 user_func_arg;
} cpu_data_t;



typedef struct _CPU_COMMON_DATA{
	tss_t tss;
	u8 isr_stack[ISR_STACK_SIZE];
} cpu_common_data_t;



static cpu_data_t* _cpu_data;
static cpu_common_data_t* _cpu_common_data;
static u8 _cpu_bsp_apic_id;
static volatile u32* _cpu_apic_ptr;

u16 cpu_count;



static void _user_func_wait_loop(void){
	while (!CPU_DATA->user_func){
		__pause();
	}
	syscall_jump_to_user_mode(CPU_DATA->user_func,CPU_DATA->user_func_arg,cpu_get_stack_top(CPU_DATA->index));
}



void _cpu_start_ap(void){
	u8 index=msr_get_apic_id();
	LOG("Initializing core #%u...",index);
	INFO("Loading IDT, GDT, TSS, FS and GS...");
	idt_enable();
	gdt_enable(&((_cpu_common_data+index)->tss));
	msr_set_fs_base(NULL);
	msr_set_gs_base(_cpu_data+index,0);
	msr_set_gs_base(NULL,1);
	INFO("Enabling SIMD...");
	msr_enable_simd();
	INFO("Enabling SYSCALL/SYSRET...");
	syscall_enable();
	INFO("Enabling RDTSC...");
	msr_enable_rdtsc();
	INFO("Enabling FSGSBASE...");
	msr_enable_fsgsbase();
	CPU_DATA->flags|=CPU_FLAG_ONLINE;
	if (index==_cpu_bsp_apic_id){
		return;
	}
	_user_func_wait_loop();
}



void cpu_init(u16 count,u64 apic_address){
	LOG("Initializing CPU manager...");
	INFO("CPU count: %u, APIC address: %p",count,apic_address);
	cpu_count=count;
	_cpu_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(count*sizeof(cpu_data_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU));
	u64 cpu_common_data_raw=pmm_alloc(pmm_align_up_address(count*sizeof(cpu_common_data_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU);
	_cpu_common_data=VMM_TRANSLATE_ADDRESS(cpu_common_data_raw);
	for (u16 i=0;i<count;i++){
		(_cpu_data+i)->index=i;
		(_cpu_data+i)->flags=0;
		(_cpu_data+i)->isr_stack_top=(u64)((_cpu_common_data+i)->isr_stack+ISR_STACK_SIZE);
		(_cpu_common_data+i)->tss.rsp0=(_cpu_data+i)->isr_stack_top;
	}
	_cpu_bsp_apic_id=msr_get_apic_id();
	_cpu_apic_ptr=VMM_TRANSLATE_ADDRESS(apic_address);
	INFO("BSP APIC id: #%u",_cpu_bsp_apic_id);
	umm_set_cpu_common_data(cpu_common_data_raw,pmm_align_up_address(count*sizeof(cpu_common_data_t))>>PAGE_SIZE_SHIFT);
	LOG("Allocating user stacks...");
	umm_set_user_stacks(pmm_alloc(cpu_count*USER_STACK_PAGE_COUNT,PMM_COUNTER_USER_STACK),cpu_count*USER_STACK_PAGE_COUNT);
}



void cpu_register_core(u8 apic_id){
	LOG("Registering CPU core #%u",apic_id);
	(_cpu_data+apic_id)->flags|=CPU_FLAG_PRESENT;
	(_cpu_data+apic_id)->user_func=0;
	(_cpu_data+apic_id)->user_func_arg=0;
}



void cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	vmm_map_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS,CPU_AP_STARTUP_MEMORY_ADDRESS,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	cpu_ap_startup_init((u32)(u64)(vmm_kernel_pagemap.toplevel));
	for (u16 i=0;i<cpu_count;i++){
		if (!((_cpu_data+i)->flags&CPU_FLAG_PRESENT)){
			ERROR("Unused CPU core: #%u",i);
			for (;;);
		}
		(_cpu_data+i)->stack_top=(u64)VMM_TRANSLATE_ADDRESS(pmm_alloc(KERNEL_STACK_PAGE_COUNT,PMM_COUNTER_KERNEL_STACK)+(KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT));
		if (i==_cpu_bsp_apic_id){
			continue;
		}
		cpu_ap_startup_set_stack_top((_cpu_data+i)->stack_top);
		_cpu_apic_ptr[160]=0;
		_cpu_apic_ptr[196]=(_cpu_apic_ptr[196]&0x00ffffff)|(i<<24);
		_cpu_apic_ptr[192]=(_cpu_apic_ptr[192]&0xfff00000)|APIC_TRIGGER_MODE_LEVEL|APIC_INTR_COMMAND_1_ASSERT|APIC_DELIVERY_MODE_INIT;
		while (_cpu_apic_ptr[192]&APIC_DELIVERY_STATUS){
			__pause();

		}
		_cpu_apic_ptr[196]=(_cpu_apic_ptr[196]&0x00ffffff)|(i<<24);
		_cpu_apic_ptr[192]=(_cpu_apic_ptr[192]&0xfff00000)|APIC_TRIGGER_MODE_LEVEL|APIC_DELIVERY_MODE_INIT;
		while (_cpu_apic_ptr[192]&APIC_DELIVERY_STATUS){
			__pause();
		}
		for (u64 j=0;j<0xfff;j++){
			__pause();
		}
		for (u8 j=0;j<2;j++){
			_cpu_apic_ptr[160]=0;
			_cpu_apic_ptr[196]=(_cpu_apic_ptr[196]&0x00ffffff)|(i<<24);
			_cpu_apic_ptr[192]=(_cpu_apic_ptr[192]&0xfff0f800)|APIC_DELIVERY_MODE_STARTUP|(CPU_AP_STARTUP_MEMORY_ADDRESS>>PAGE_SIZE_SHIFT);
			for (u64 j=0;j<0xfff;j++){
				__pause();
			}
			while (_cpu_apic_ptr[192]&APIC_DELIVERY_STATUS){
				__pause();
			}
		}
		const volatile u8* flags=&((_cpu_data+i)->flags);
		while (!((*flags)&CPU_FLAG_ONLINE)){
			__pause();
		}
	}
	vmm_unmap_page(&vmm_kernel_pagemap,CPU_AP_STARTUP_MEMORY_ADDRESS);
	_cpu_start_ap();
}



void cpu_start_program(void* start_address){
	if (CPU_DATA->index!=_cpu_bsp_apic_id){
		ERROR("Unable to start program from non-bsp CPU");
		return;
	}
	syscall_jump_to_user_mode((u64)start_address,0,cpu_get_stack_top(CPU_DATA->index));
}



void cpu_core_stop(void){
	if (CPU_DATA->index){
		CPU_DATA->user_func=0;
		_user_func_wait_loop();
	}
	acpi_fadt_shutdown(0);
}
