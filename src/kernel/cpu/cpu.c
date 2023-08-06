#include <kernel/acpi/fadt.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/ap_startup.h>
#include <kernel/cpu/cpu.h>
#include <kernel/elf/elf.h>
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



static cpu_data_t* _cpu_data;
static cpu_common_data_t* _cpu_common_data;
static volatile u32* _cpu_apic_ptr;

u16 cpu_count;
u8 cpu_bsp_core_id;



void KERNEL_NORETURN _cpu_init_core(void){
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
	syscall_jump_to_user_mode();
}



void cpu_init(u16 count,u64 apic_address){
	LOG("Initializing CPU manager...");
	INFO("CPU count: %u, APIC address: %p",count,apic_address);
	cpu_count=count;
	_cpu_data=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(count*sizeof(cpu_data_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU));
	u64 cpu_common_data_raw=pmm_alloc(pmm_align_up_address(count*sizeof(cpu_common_data_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_CPU);
	_cpu_common_data=VMM_TRANSLATE_ADDRESS(cpu_common_data_raw);
	u64 user_stacks=pmm_alloc(cpu_count*CPU_USER_STACK_PAGE_COUNT,PMM_COUNTER_USER_STACK);
	u64 kernel_stacks=pmm_alloc(cpu_count*CPU_KERNEL_STACK_PAGE_COUNT,PMM_COUNTER_KERNEL_STACK);
	for (u16 i=0;i<count;i++){
		kernel_stacks+=CPU_KERNEL_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT;
		(_cpu_data+i)->index=i;
		(_cpu_data+i)->flags=0;
		(_cpu_data+i)->kernel_rsp=(u64)VMM_TRANSLATE_ADDRESS(kernel_stacks);
		(_cpu_data+i)->isr_rsp=(u64)((_cpu_common_data+i)->isr_stack+ISR_STACK_SIZE);
		(_cpu_data+i)->user_func=0;
		(_cpu_data+i)->user_func_arg=0;
		(_cpu_data+i)->user_rsp_top=UMM_STACK_TOP-i*(CPU_USER_STACK_PAGE_COUNT<<PAGE_SIZE_SHIFT);
		(_cpu_common_data+i)->tss.rsp0=(_cpu_data+i)->isr_rsp;
	}
	cpu_bsp_core_id=msr_get_apic_id();
	_cpu_apic_ptr=VMM_TRANSLATE_ADDRESS(apic_address);
	INFO("BSP APIC id: #%u",cpu_bsp_core_id);
	umm_set_cpu_common_data(cpu_common_data_raw,pmm_align_up_address(count*sizeof(cpu_common_data_t))>>PAGE_SIZE_SHIFT);
	umm_set_user_stacks(user_stacks,cpu_count*CPU_USER_STACK_PAGE_COUNT);
}



void cpu_register_core(u8 apic_id){
	LOG("Registering CPU core #%u",apic_id);
	(_cpu_data+apic_id)->flags|=CPU_FLAG_PRESENT;
}



void cpu_start_all_cores(void){
	LOG("Starting all cpu cores...");
	cpu_ap_startup_init();
	for (u16 i=0;i<cpu_count;i++){
		if (!((_cpu_data+i)->flags&CPU_FLAG_PRESENT)){
			ERROR("Unused CPU core: #%u",i);
			for (;;);
		}
		if (i==cpu_bsp_core_id){
			continue;
		}
		cpu_ap_startup_set_stack_top((_cpu_data+i)->kernel_rsp);
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
			while (_cpu_apic_ptr[192]&APIC_DELIVERY_STATUS){
				__pause();
			}
		}
		const volatile u8* flags=&((_cpu_data+i)->flags);
		while (!((*flags)&CPU_FLAG_ONLINE)){
			__pause();
		}
	}
	cpu_ap_startup_deinit();
	volatile cpu_data_t* cpu_data=_cpu_data+cpu_bsp_core_id;
	cpu_data->user_func_arg=0;
	cpu_data->user_func=elf_load("/loader.elf");
	_cpu_init_core();
}



void cpu_core_start(u8 index,u64 start_address,u64 arg){
	volatile cpu_data_t* cpu_data=_cpu_data+index;
	cpu_data->user_func_arg=arg;
	cpu_data->user_func=start_address;
	if (CPU_DATA->index==index){
		syscall_jump_to_user_mode();
	}
}



void KERNEL_NORETURN cpu_core_stop(void){
	if (CPU_DATA->index==cpu_bsp_core_id){
		CPU_DATA->user_func_arg=0;
		CPU_DATA->user_func=elf_load("/loader.elf");
	}
	syscall_jump_to_user_mode();
}
