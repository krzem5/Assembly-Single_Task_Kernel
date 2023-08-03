#include <kernel/acpi/fadt.h>
#include <kernel/clock/clock.h>
#include <kernel/context/context.h>
#include <kernel/cpu/cpu.h>
#include <kernel/drive/drive.h>
#include <kernel/drive/drive_list.h>
#include <kernel/elf/elf.h>
#include <kernel/fd/fd.h>
#include <kernel/fs/fs.h>
#include <kernel/fs_provider/kfs.h>
#include <kernel/log/log.h>
#include <kernel/memory/memcpy.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/network/layer2.h>
#include <kernel/serial/serial.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "syscall"



#define SYSCALL_COUNT 28



#define USER_DRIVE_FLAG_PRESENT 1
#define USER_DRIVE_FLAG_BOOT 2

#define USER_PARTITION_FLAG_PRESENT 1
#define USER_PARTITION_FLAG_BOOT 2
#define USER_PARTITION_FLAG_HALF_INSTALLED 4
#define USER_PARTITION_FLAG_PREVIOUS_BOOT 8

#define USER_SHUTDOWN_FLAG_RESTART 1
#define USER_SHUTDOWN_FLAG_SAVE_CONTEXT 2



typedef struct _USER_DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[16];
	char serial_number[32];
	char model_number[64];
	u64 block_count;
	u64 block_size;
} user_drive_t;



typedef struct _USER_PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char name[16];
	u32 drive_index;
} user_partition_t;



void* _syscall_handlers[SYSCALL_COUNT+1];



static void syscall_serial_send(syscall_registers_t* regs){
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		return;
	}
	serial_send(VMM_TRANSLATE_ADDRESS(address),regs->rsi);
}



static void syscall_serial_recv(syscall_registers_t* regs){
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	regs->rax=serial_recv(VMM_TRANSLATE_ADDRESS(address),regs->rsi,regs->rdx);
}



static void syscall_elf_load(syscall_registers_t* regs){
	regs->rax=-1;
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		return;
	}
	char buffer[4096];
	if (regs->rsi>4095){
		return;
	}
	memcpy(buffer,VMM_TRANSLATE_ADDRESS(address),regs->rsi);
	buffer[regs->rsi]=0;
	void* start_address=elf_load(buffer);
	if (!start_address){
		return;
	}
	cpu_start_program(start_address);
}



static void syscall_cpu_core_count(syscall_registers_t* regs){
	regs->rax=cpu_get_core_count();
}



static void syscall_cpu_core_start(syscall_registers_t* regs){
	WARN("Unimplemented: syscall_cpu_core_start");
}



static void syscall_cpu_core_stop(syscall_registers_t* regs){
	cpu_core_stop();
}



static void syscall_drive_list_length(syscall_registers_t* regs){
	regs->rax=drive_list_get_length();
}



static void syscall_drive_list_get(syscall_registers_t* regs){
	const drive_t* drive=drive_list_get_drive(regs->rdi);
	if (!drive||regs->rdx!=sizeof(user_drive_t)){
		regs->rax=-1;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=-1;
		return;
	}
	user_drive_t* user_drive=VMM_TRANSLATE_ADDRESS(address);
	user_drive->flags=USER_DRIVE_FLAG_PRESENT|((drive->flags&DRIVE_FLAG_BOOT)?USER_DRIVE_FLAG_BOOT:0);
	user_drive->type=drive->type;
	user_drive->index=regs->rdi;
	memcpy(user_drive->name,drive->name,16);
	memcpy(user_drive->serial_number,drive->serial_number,32);
	memcpy(user_drive->model_number,drive->model_number,64);
	user_drive->block_count=drive->block_count;
	user_drive->block_size=drive->block_size;
	regs->rax=0;
}



static void syscall_file_system_count(syscall_registers_t* regs){
	regs->rax=partition_count;
}



static void syscall_file_system_get(syscall_registers_t* regs){
	if (regs->rdi>=partition_count||regs->rdx!=sizeof(user_partition_t)){
		regs->rax=-1;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=-1;
		return;
	}
	const fs_partition_t* partition=partition_data+regs->rdi;
	user_partition_t* user_partition=VMM_TRANSLATE_ADDRESS(address);
	user_partition->flags=USER_PARTITION_FLAG_PRESENT|((partition->flags&FS_PARTITION_FLAG_BOOT)?USER_PARTITION_FLAG_BOOT:0)|((partition->flags&FS_PARTITION_FLAG_HALF_INSTALLED)?USER_PARTITION_FLAG_HALF_INSTALLED:0)|((partition->flags&FS_PARTITION_FLAG_PREVIOUS_BOOT)?USER_PARTITION_FLAG_PREVIOUS_BOOT:0);
	user_partition->type=partition->partition_config.type;
	user_partition->index=partition->partition_config.index;
	user_partition->first_block_index=partition->partition_config.first_block_index;
	user_partition->last_block_index=partition->partition_config.last_block_index;
	memcpy(user_partition->name,partition->name,16);
	user_partition->drive_index=partition->drive->index;
	regs->rax=0;
}



static void syscall_fd_open(syscall_registers_t* regs){
	if (regs->rdi&&FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_open(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx,regs->r8);
}



static void syscall_fd_close(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_close(regs->rdi);
}



static void syscall_fd_delete(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_delete(regs->rdi);
}



static void syscall_fd_read(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_read(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx);
}



static void syscall_fd_write(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_write(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx);
}



static void syscall_fd_seek(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_seek(regs->rdi,regs->rsi,regs->rdx);
}



static void syscall_fd_stat(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	if (regs->rdx!=sizeof(fd_stat_t)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_stat(regs->rdi,VMM_TRANSLATE_ADDRESS(address));
}



static void syscall_fd_get_relative(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_get_relative(regs->rdi,regs->rsi,regs->rdx);
}



static void syscall_fd_move(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)||FD_OUT_OF_RANGE(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_move(regs->rdi,regs->rsi);
}



static void syscall_net_send(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(network_layer2_packet_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	network_layer2_packet_t packet=*((const network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address));
	u64 buffer_address=syscall_sanatize_user_memory((u64)(packet.buffer),packet.buffer_length);
	if (!buffer_address){
		regs->rax=0;
		return;
	}
	packet.buffer=VMM_TRANSLATE_ADDRESS(buffer_address);
	regs->rax=network_layer2_send(&packet);
}



static void syscall_net_poll(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(network_layer2_packet_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	network_layer2_packet_t packet=*((network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address));
	void* user_buffer=packet.buffer;
	u64 buffer_address=syscall_sanatize_user_memory((u64)user_buffer,packet.buffer_length);
	if (!buffer_address){
		regs->rax=0;
		return;
	}
	packet.buffer=VMM_TRANSLATE_ADDRESS(buffer_address);
	regs->rax=network_layer2_poll(&packet);
	packet.buffer=user_buffer;
	*((network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address))=packet;
}



static void syscall_system_shutdown(syscall_registers_t* regs){
	if (regs->rdi&USER_SHUTDOWN_FLAG_SAVE_CONTEXT){
		context_save();
	}
	acpi_fadt_shutdown(!!(regs->rdi&USER_SHUTDOWN_FLAG_RESTART));
}



static void syscall_memory_map(syscall_registers_t* regs){
	regs->rax=mmap_alloc(regs->rdi,regs->rsi);
}



static void syscall_memory_unmap(syscall_registers_t* regs){
	regs->rax=mmap_dealloc(regs->rdi,regs->rsi);
}



static void syscall_memory_stats(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(pmm_counters_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	*((pmm_counters_t*)VMM_TRANSLATE_ADDRESS(address))=*pmm_get_counters();
	regs->rax=1;
}



static void syscall_clock_get_converion(syscall_registers_t* regs){
	regs->rax=clock_conversion_factor;
	regs->rdx=clock_conversion_shift;
	regs->r8=clock_cpu_frequency;
}



static void syscall_drive_format(syscall_registers_t* regs){
	const drive_t* drive=drive_list_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	u64 address=0;
	if (regs->rdx){
		address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
		if (!address){
			regs->rax=0;
			return;
		}
		address=(u64)VMM_TRANSLATE_ADDRESS(address);
	}
	regs->rax=kfs_format_drive(drive,(void*)address,regs->rdx);
}



static void syscall_drive_stats(syscall_registers_t* regs){
	const drive_t* drive=drive_list_get_drive(regs->rdi);
	if (!drive||regs->rdx!=sizeof(drive_stats_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=0;
		return;
	}
	fs_partition_flush_cache();
	*((drive_stats_t*)VMM_TRANSLATE_ADDRESS(address))=*(drive->stats);
	regs->rax=1;
}



static void syscall_invalid(syscall_registers_t* regs,u64 number){
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
	_syscall_handlers[8]=syscall_file_system_count;
	_syscall_handlers[9]=syscall_file_system_get;
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
	_syscall_handlers[SYSCALL_COUNT]=syscall_invalid;
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
