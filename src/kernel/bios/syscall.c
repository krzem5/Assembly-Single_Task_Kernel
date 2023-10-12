#include <kernel/bios/bios.h>
#include <kernel/syscall/syscall.h>



#define SYSTEM_STRING_BIOS_VENDOR 0
#define SYSTEM_STRING_BIOS_VERSION 1
#define SYSTEM_STRING_MANUFACTURER 2
#define SYSTEM_STRING_PRODUCT 3
#define SYSTEM_STRING_VERSION 4
#define SYSTEM_STRING_SERIAL_NUMBER 5
#define SYSTEM_STRING_UUID 6
#define SYSTEM_STRING_LAST_WAKEUP 7



void syscall_system_get_string(syscall_registers_t* regs){
	if (regs->rsi&&!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const char* src=NULL;
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
			switch (bios_data.wakeup_type){
				case BIOS_DATA_WAKEUP_TYPE_UNKNOWN:
					src="Unknown";
					break;
				case BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH:
					src="Power switch";
					break;
				case BIOS_DATA_WAKEUP_TYPE_AC_POWER:
					src="AC power";
					break;
			}
			break;
	}
	if (!src){
		regs->rax=0;
		return;
	}
	if (!regs->rsi){
		for (regs->rax=0;src[regs->rax];regs->rax++);
		return;
	}
	if (!regs->rdx){
		regs->rax=0;
		return;
	}
	char* out=(void*)(regs->rsi);
	regs->rax=0;
	while (regs->rax<regs->rdx-1&&src[regs->rax]){
		out[regs->rax]=src[regs->rax];
		regs->rax++;
	}
	out[regs->rax]=0;
}
