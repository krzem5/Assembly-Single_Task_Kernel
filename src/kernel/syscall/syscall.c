#include <kernel/acpi/syscall.h>
#include <kernel/clock/syscall.h>
#include <kernel/context/syscall.h>
#include <kernel/cpu/syscall.h>
#include <kernel/drive/syscall.h>
#include <kernel/elf/syscall.h>
#include <kernel/fd/syscall.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/syscall.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/syscall.h>
#include <kernel/partition/syscall.h>
#include <kernel/serial/syscall.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "syscall"



#define SYSCALL_COUNT 29



void* _syscall_handlers[SYSCALL_COUNT+1];



static void _syscall_invalid(syscall_registers_t* regs,u64 number){
	ERROR("Invalid SYSCALL number: %lu",number);
	for (;;);
}



void syscall_init(void){
	LOG("Initializing SYSCALL table...");
	_syscall_handlers[0]=syscall_serial_send;
	_syscall_handlers[1]=syscall_serial_recv;
	_syscall_handlers[2]=syscall_elf_load;
	_syscall_handlers[3]=syscall_cpu_core_count;
	_syscall_handlers[4]=syscall_cpu_core_start;
	_syscall_handlers[5]=syscall_cpu_core_stop;
	_syscall_handlers[6]=syscall_drive_list_length;
	_syscall_handlers[7]=syscall_drive_list_get;
	_syscall_handlers[8]=syscall_partition_count;
	_syscall_handlers[9]=syscall_partition_get;
	_syscall_handlers[10]=syscall_fd_open;
	_syscall_handlers[11]=syscall_fd_close;
	_syscall_handlers[12]=syscall_fd_delete;
	_syscall_handlers[13]=syscall_fd_read;
	_syscall_handlers[14]=syscall_fd_write;
	_syscall_handlers[15]=syscall_fd_seek;
	_syscall_handlers[16]=syscall_fd_stat;
	_syscall_handlers[17]=syscall_fd_get_relative;
	_syscall_handlers[18]=syscall_fd_move;
	_syscall_handlers[19]=syscall_net_send;
	_syscall_handlers[20]=syscall_net_poll;
	_syscall_handlers[21]=syscall_system_shutdown;
	_syscall_handlers[22]=syscall_memory_map;
	_syscall_handlers[23]=syscall_memory_unmap;
	_syscall_handlers[24]=syscall_memory_stats;
	_syscall_handlers[25]=syscall_clock_get_converion;
	_syscall_handlers[26]=syscall_drive_format;
	_syscall_handlers[27]=syscall_drive_stats;
	_syscall_handlers[28]=syscall_context_save;
	_syscall_handlers[SYSCALL_COUNT]=_syscall_invalid;
}



u64 syscall_sanatize_user_memory(u64 start,u64 size){
	u64 address=vmm_virtual_to_physical(&vmm_user_pagemap,start);
	if (!address||!size){
		return 0;
	}
	for (u64 offset=PAGE_SIZE;offset<size;offset+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(&vmm_user_pagemap,start+offset)){
			return 0;
		}
	}
	return address;
}
