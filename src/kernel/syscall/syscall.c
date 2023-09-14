#include <kernel/acpi/syscall.h>
#include <kernel/clock/syscall.h>
#include <kernel/coverage/syscall.h>
#include <kernel/cpu/cpu.h>
#include <kernel/cpu/syscall.h>
#include <kernel/drive/syscall.h>
#include <kernel/elf/syscall.h>
#include <kernel/fd/syscall.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/syscall.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/syscall.h>
#include <kernel/random/syscall.h>
#include <kernel/serial/syscall.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/user/syscall.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "syscall"



void syscall_empty(syscall_registers_t* regs){
}



void syscall_invalid(syscall_registers_t* regs){
	ERROR("Invalid SYSCALL number: %lu",regs->rax);
	panic("Invalid SYSCALL",1);
}



void* _syscall_handlers[]={
	[0]=syscall_empty,
	[1]=syscall_serial_send,
	[2]=syscall_serial_recv,
	[3]=syscall_elf_load,
	[4]=syscall_cpu_core_start,
	[5]=syscall_cpu_core_stop,
	[6]=syscall_fd_open,
	[7]=syscall_fd_close,
	[8]=syscall_fd_delete,
	[9]=syscall_fd_read,
	[10]=syscall_fd_write,
	[11]=syscall_fd_seek,
	[12]=syscall_fd_resize,
	[13]=syscall_fd_absolute_path,
	[14]=syscall_fd_stat,
	[15]=syscall_fd_get_relative,
	[16]=syscall_fd_move,
	[17]=syscall_network_layer2_send,
	[18]=syscall_network_layer2_poll,
	[19]=syscall_network_layer3_refresh,
	[20]=syscall_network_layer3_device_count,
	[21]=syscall_network_layer3_device_get,
	[22]=syscall_network_layer3_device_delete,
	[23]=syscall_system_shutdown,
	[24]=syscall_memory_map,
	[25]=syscall_memory_unmap,
	[26]=syscall_memory_stats,
	[27]=syscall_clock_get_converion,
	[28]=syscall_drive_format,
	[29]=syscall_drive_stats,
	[30]=syscall_random_generate,
	[31]=syscall_coverage_dump_data,
	[32]=syscall_user_data_pointer,
	NULL
};



u64 syscall_sanatize_user_memory(u64 start,u64 size){
	for (;;);
	vmm_pagemap_t* pagemap=&(CPU_DATA->scheduler->current_thread->process->pagemap);
	u64 address=vmm_virtual_to_physical(pagemap,start);
	if (!address||!size){
		return 0;
	}
	for (u64 offset=PAGE_SIZE;offset<size;offset+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(pagemap,start+offset)){
			return 0;
		}
	}
	return address;
}
