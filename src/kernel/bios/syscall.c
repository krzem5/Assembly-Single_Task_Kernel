#include <kernel/bios/bios.h>
#include <kernel/isr/isr.h>
#include <kernel/memory/smm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/util/util.h>



#define SYSTEM_STRING_BIOS_VENDOR 0
#define SYSTEM_STRING_BIOS_VERSION 1
#define SYSTEM_STRING_MANUFACTURER 2
#define SYSTEM_STRING_PRODUCT 3
#define SYSTEM_STRING_VERSION 4
#define SYSTEM_STRING_SERIAL_NUMBER 5
#define SYSTEM_STRING_UUID 6
#define SYSTEM_STRING_LAST_WAKEUP 7



void syscall_system_get_string(isr_state_t* regs){
	if (regs->rsi&&!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const string_t* src=NULL;
	switch (regs->rdi){
		case SYSTEM_STRING_BIOS_VENDOR:
			src=bios_data.bios_vendor;
			break;
		case SYSTEM_STRING_BIOS_VERSION:
			src=bios_data.bios_version;
			break;
		case SYSTEM_STRING_MANUFACTURER:
			src=bios_data.manufacturer;
			break;
		case SYSTEM_STRING_PRODUCT:
			src=bios_data.product;
			break;
		case SYSTEM_STRING_VERSION:
			src=bios_data.version;
			break;
		case SYSTEM_STRING_SERIAL_NUMBER:
			src=bios_data.serial_number;
			break;
		case SYSTEM_STRING_UUID:
			src=bios_data.uuid_str;
			break;
		case SYSTEM_STRING_LAST_WAKEUP:
			src=bios_data.wakeup_type_str;
			break;
	}
	if (!src){
		regs->rax=0;
		return;
	}
	if (!regs->rsi){
		regs->rax=src->length;
		return;
	}
	if (!regs->rdx){
		regs->rax=0;
		return;
	}
	regs->rax=regs->rdx-1;
	if (regs->rax>src->length){
		regs->rax=src->length;
	}
	char* out=(void*)(regs->rsi);
	memcpy(out,src->data,regs->rax);
	out[regs->rax]=0;
}
