#include <kernel/acpi/syscall.h>
#include <kernel/clock/syscall.h>
#include <kernel/coverage/syscall.h>
#include <kernel/cpu/cpu.h>
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
#include <kernel/thread/syscall.h>
#include <kernel/types.h>
#include <kernel/user/syscall.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "syscall"



void syscall_invalid(syscall_registers_t* regs){
	ERROR("Invalid SYSCALL number: %lu",regs->rax);
	panic("Invalid SYSCALL",1);
}



void* _syscall_handlers[]={
	[0]=syscall_invalid,
	[1]=syscall_serial_send,
	[2]=syscall_serial_recv,
	[3]=syscall_elf_load,
	[4]=syscall_fd_open,
	[5]=syscall_fd_close,
	[6]=syscall_fd_delete,
	[7]=syscall_fd_read,
	[8]=syscall_fd_write,
	[9]=syscall_fd_seek,
	[10]=syscall_fd_resize,
	[11]=syscall_fd_absolute_path,
	[12]=syscall_fd_stat,
	[13]=syscall_fd_get_relative,
	[14]=syscall_fd_move,
	[15]=syscall_network_layer2_send,
	[16]=syscall_network_layer2_poll,
	[17]=syscall_network_layer3_refresh,
	[18]=syscall_network_layer3_device_count,
	[19]=syscall_network_layer3_device_get,
	[20]=syscall_network_layer3_device_delete,
	[21]=syscall_system_shutdown,
	[22]=syscall_memory_map,
	[23]=syscall_memory_unmap,
	[24]=syscall_memory_stats,
	[25]=syscall_clock_get_converion,
	[26]=syscall_drive_format,
	[27]=syscall_drive_stats,
	[28]=syscall_random_generate,
	[29]=syscall_coverage_dump_data,
	[30]=syscall_user_data_pointer,
	[31]=syscall_thread_stop,
	[32]=syscall_thread_create,
	NULL
};



_Bool syscall_sanatize_user_memory(u64 address,u64 size){
	if (!address||!size||(address|size|(address+size))>=VMM_HIGHER_HALF_ADDRESS_OFFSET){
		return 0;
	}
	for (u64 offset=0;offset<size;offset+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(&(CPU_HEADER_DATA->cpu_data->scheduler->current_thread->process->user_pagemap),address+offset)){
			return 0;
		}
	}
	return 1;
}
