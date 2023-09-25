#include <kernel/acpi/syscall.h>
#include <kernel/clock/syscall.h>
#include <kernel/coverage/syscall.h>
#include <kernel/cpu/cpu.h>
#include <kernel/drive/syscall.h>
#include <kernel/elf/syscall.h>
#include <kernel/fd/syscall.h>
#include <kernel/handle/syscall.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/syscall.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/syscall.h>
#include <kernel/mp/thread.h>
#include <kernel/network/syscall.h>
#include <kernel/random/syscall.h>
#include <kernel/serial/syscall.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/user/syscall.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "syscall"



void syscall_invalid(syscall_registers_t* regs){
	ERROR("Invalid SYSCALL number: %lu",regs->rax);
	panic("Invalid SYSCALL");
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
	[24]=syscall_memory_get_counter_count,
	[25]=syscall_memory_get_counter,
	[26]=syscall_clock_get_converion,
	[27]=syscall_drive_format,
	[28]=syscall_drive_stats,
	[29]=syscall_random_generate,
	[30]=syscall_coverage_dump_data,
	[31]=syscall_user_data_pointer,
	[32]=syscall_thread_stop,
	[33]=syscall_thread_create,
	[34]=syscall_thread_get_priority,
	[35]=syscall_thread_set_priority,
	[36]=syscall_handle_get_type_count,
	[37]=syscall_handle_get_type,
	NULL
};



_Bool syscall_sanatize_user_memory(u64 address,u64 size){
	if (!address||!size||(address|size|(address+size))>=VMM_HIGHER_HALF_ADDRESS_OFFSET){
		return 0;
	}
	for (u64 offset=0;offset<size;offset+=PAGE_SIZE){
		if (!vmm_virtual_to_physical(&(THREAD_DATA->process->pagemap),address+offset)){
			return 0;
		}
	}
	return 1;
}
