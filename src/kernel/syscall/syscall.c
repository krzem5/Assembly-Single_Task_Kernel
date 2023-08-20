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
#include <kernel/numa/syscall.h>
#include <kernel/partition/syscall.h>
#include <kernel/random/syscall.h>
#include <kernel/serial/syscall.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "syscall"



void syscall_empty(syscall_registers_t* regs){
}



void syscall_dump_coverage_data(syscall_registers_t* regs);



void KERNEL_NORETURN syscall_invalid(syscall_registers_t* regs){
	ERROR("Invalid SYSCALL number: %lu",regs->rax);
	for (;;);
}



void* _syscall_handlers[]={
	[0]=syscall_empty,
	[1]=syscall_serial_send,
	[2]=syscall_serial_recv,
	[3]=syscall_elf_load,
	[4]=syscall_cpu_core_count,
	[5]=syscall_cpu_core_start,
	[6]=syscall_cpu_core_stop,
	[7]=syscall_drive_list_length,
	[8]=syscall_drive_list_get,
	[9]=syscall_partition_count,
	[10]=syscall_partition_get,
	[11]=syscall_fd_open,
	[12]=syscall_fd_close,
	[13]=syscall_fd_delete,
	[14]=syscall_fd_read,
	[15]=syscall_fd_write,
	[16]=syscall_fd_seek,
	[17]=syscall_fd_resize,
	[18]=syscall_fd_absolute_path,
	[19]=syscall_fd_stat,
	[20]=syscall_fd_get_relative,
	[21]=syscall_fd_move,
	[22]=syscall_network_layer1_config,
	[23]=syscall_network_layer2_send,
	[24]=syscall_network_layer2_poll,
	[25]=syscall_network_layer3_refresh,
	[26]=syscall_network_layer3_device_count,
	[27]=syscall_network_layer3_device_get,
	[28]=syscall_network_layer3_device_delete,
	[29]=syscall_system_shutdown,
	[30]=syscall_system_config,
	[31]=syscall_memory_map,
	[32]=syscall_memory_unmap,
	[33]=syscall_memory_stats,
	[34]=syscall_clock_get_converion,
	[35]=syscall_drive_format,
	[36]=syscall_drive_stats,
	[37]=syscall_random_generate,
	[38]=syscall_numa_node_count,
#if KERNEL_COVERAGE_ENABLED
	[39]=syscall_dump_coverage_data,
#endif
	NULL
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
