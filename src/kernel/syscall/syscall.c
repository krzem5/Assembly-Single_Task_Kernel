#include <kernel/acpi/syscall.h>
#include <kernel/bios/syscall.h>
#include <kernel/clock/syscall.h>
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
#include <kernel/random/syscall.h>
#include <kernel/serial/syscall.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "syscall"



#define SYSCALL_COUNT 37



static void _syscall_invalid(syscall_registers_t* regs,u64 number){
	ERROR("Invalid SYSCALL number: %lu",number);
	for (;;);
}



void* _syscall_handlers[SYSCALL_COUNT+1]={
	[0]=syscall_serial_send,
	[1]=syscall_serial_recv,
	[2]=syscall_elf_load,
	[3]=syscall_cpu_core_count,
	[4]=syscall_cpu_core_start,
	[5]=syscall_cpu_core_stop,
	[6]=syscall_drive_list_length,
	[7]=syscall_drive_list_get,
	[8]=syscall_partition_count,
	[9]=syscall_partition_get,
	[10]=syscall_fd_open,
	[11]=syscall_fd_close,
	[12]=syscall_fd_delete,
	[13]=syscall_fd_read,
	[14]=syscall_fd_write,
	[15]=syscall_fd_seek,
	[16]=syscall_fd_resize,
	[17]=syscall_fd_absolute_path,
	[18]=syscall_fd_stat,
	[19]=syscall_fd_get_relative,
	[20]=syscall_fd_move,
	[21]=syscall_network_layer1_config,
	[22]=syscall_network_layer2_send,
	[23]=syscall_network_layer2_poll,
	[24]=syscall_network_layer3_refresh,
	[25]=syscall_network_layer3_device_count,
	[26]=syscall_network_layer3_device_get,
	[27]=syscall_network_layer3_device_delete,
	[28]=syscall_system_shutdown,
	[29]=syscall_system_config,
	[30]=syscall_memory_map,
	[31]=syscall_memory_unmap,
	[32]=syscall_memory_stats,
	[33]=syscall_clock_get_converion,
	[34]=syscall_drive_format,
	[35]=syscall_drive_stats,
	[36]=syscall_random_generate,
	[SYSCALL_COUNT]=_syscall_invalid
};



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
