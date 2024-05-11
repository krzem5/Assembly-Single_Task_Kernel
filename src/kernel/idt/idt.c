#include <kernel/idt/idt.h>
#include <kernel/isr/isr.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "idt"



typedef struct KERNEL_PACKED _IDT_ENTRY{
	u16 offset_low;
	u32 flags;
	u16 offset_medium;
	u64 offset_high;
} idt_entry_t;



KERNEL_EARLY_EARLY_INIT(){
	LOG("Initializing IDT...");
	u64 addr=pmm_alloc(pmm_align_up_address(256*sizeof(idt_entry_t))>>PAGE_SIZE_SHIFT,pmm_alloc_counter("kernel.idt"),0);
	idt_entry_t* entries=(void*)(addr+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u32 i=0;i<256;i++){
		u64 handler=_isr_handler_list[i];
		u8 ist=0;
		if (i==14){
			ist=1;
		}
		else if (i==32){
			ist=2;
		}
		(entries+i)->offset_low=handler;
		(entries+i)->flags=0x8e000008|(ist<<16);
		(entries+i)->offset_medium=handler>>16;
		(entries+i)->offset_high=handler>>32;
	}
	_idt_set_data_pointer(addr+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}
