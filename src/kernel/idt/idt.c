#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "idt"



extern u64 _idt_data[];



void idt_set_entry(u8 index,void* callback,u8 ist,u8 flags){
	u64 callback_address=(u64)callback;
	_idt_data[index<<1]=((callback_address&0xffff0000)<<32)|(((u64)flags)<<40)|(((u64)ist)<<32)|0x00080000|(callback_address&0x0000ffff);
	_idt_data[(index<<1)|1]=callback_address>>32;
}
