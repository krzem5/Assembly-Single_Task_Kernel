#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/umm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "idt"



void* _idt_data;



void idt_init(void){
	LOG("Initializing IDT...");
	u64 idt_data_raw=pmm_alloc_zero(1,PMM_COUNTER_CPU);
	_idt_data=VMM_TRANSLATE_ADDRESS(idt_data_raw);
	umm_set_idt_data(idt_data_raw);
}



void idt_set_entry(u8 index,void* callback,u8 ist,u8 flags);
