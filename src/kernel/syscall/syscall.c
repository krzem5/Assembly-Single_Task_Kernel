#include <kernel/acpi/fadt.h>
#include <kernel/clock/clock.h>
#include <kernel/cpu/cpu.h>
#include <kernel/drive/drive.h>
#include <kernel/drive/drive_list.h>
#include <kernel/elf/elf.h>
#include <kernel/fs/fd.h>
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



#define SYSCALL_COUNT 27



#define USER_DRIVE_FLAG_PRESENT 1
#define USER_DRIVE_FLAG_BOOT 2

#define USER_PARTITION_FLAG_PRESENT 1
#define USER_PARTITION_FLAG_BOOT 2



typedef struct _SYSCALL_REGISTERS{
	u64 rax;
	u64 rbx;
	u64 rdx;
	u64 rsi;
	u64 rdi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;
	u64 rflags;
	u64 rip;
} syscall_registers_t;



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



static u64 _sanatize_user_memory(u64 start,u64 size){
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



static void _syscall_serial_send(syscall_registers_t* regs){
	u64 address=_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		return;
	}
	serial_send(VMM_TRANSLATE_ADDRESS(address),regs->rsi);
}



static void _syscall_serial_recv(syscall_registers_t* regs){
	u64 address=_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	regs->rax=serial_recv(VMM_TRANSLATE_ADDRESS(address),regs->rsi,regs->rdx);
}



static void _syscall_elf_load(syscall_registers_t* regs){
	regs->rax=-1;
	u64 address=_sanatize_user_memory(regs->rdi,regs->rsi);
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



static void _syscall_cpu_core_count(syscall_registers_t* regs){
	regs->rax=cpu_get_core_count();
}



static void _syscall_cpu_core_start(syscall_registers_t* regs){
	// int _syscall_cpu_core_start(unsigned int index,void* fn,void* arg);
	WARN("Unimplemented: _syscall_cpu_core_start");
}



static void _syscall_cpu_core_stop(syscall_registers_t* regs){
	cpu_core_stop();
}



static void _syscall_drive_list_length(syscall_registers_t* regs){
	regs->rax=drive_list_get_length();
}



static void _syscall_drive_list_get(syscall_registers_t* regs){
	const drive_t* drive=drive_list_get_drive(regs->rdi);
	if (!drive||regs->rdx!=sizeof(user_drive_t)){
		regs->rax=-1;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
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



static void _syscall_file_system_count(syscall_registers_t* regs){
	regs->rax=fs_get_file_system_count();
}



static void _syscall_file_system_get(syscall_registers_t* regs){
	const fs_file_system_t* file_system=fs_get_file_system(regs->rdi);
	if (!file_system||regs->rdx!=sizeof(user_partition_t)){
		regs->rax=-1;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=-1;
		return;
	}
	user_partition_t* user_partition=VMM_TRANSLATE_ADDRESS(address);
	user_partition->flags=USER_PARTITION_FLAG_PRESENT|(fs_get_boot_file_system()==regs->rdi?USER_PARTITION_FLAG_BOOT:0);
	user_partition->type=file_system->partition_config.type;
	user_partition->index=file_system->partition_config.index;
	user_partition->first_block_index=file_system->partition_config.first_block_index;
	user_partition->last_block_index=file_system->partition_config.last_block_index;
	memcpy(user_partition->name,file_system->name,16);
	user_partition->drive_index=file_system->drive->index;
	regs->rax=0;
}



static void _syscall_fd_open(syscall_registers_t* regs){
	if (regs->rdi&&FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_open(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx,regs->r8);
}



static void _syscall_fd_close(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_close(regs->rdi);
}



static void _syscall_fd_delete(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_delete(regs->rdi);
}



static void _syscall_fd_read(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_read(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx);
}



static void _syscall_fd_write(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_write(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx);
}



static void _syscall_fd_seek(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_seek(regs->rdi,regs->rsi,regs->rdx);
}



static void _syscall_fd_stat(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	if (regs->rdx!=sizeof(fd_stat_t)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_stat(regs->rdi,VMM_TRANSLATE_ADDRESS(address));
}



static void _syscall_fd_get_relative(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_get_relative(regs->rdi,regs->rsi,regs->rdx);
}



static void _syscall_fd_move(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)||FD_OUT_OF_RANGE(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_move(regs->rdi,regs->rsi);
}



static void _syscall_net_send(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(network_layer2_packet_t)){
		regs->rax=0;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	network_layer2_packet_t packet=*((const network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address));
	u64 buffer_address=_sanatize_user_memory((u64)(packet.buffer),packet.buffer_length);
	if (!buffer_address){
		regs->rax=0;
		return;
	}
	packet.buffer=VMM_TRANSLATE_ADDRESS(buffer_address);
	regs->rax=network_layer2_send(&packet);
}



static void _syscall_net_poll(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(network_layer2_packet_t)){
		regs->rax=0;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	network_layer2_packet_t packet=*((network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address));
	void* user_buffer=packet.buffer;
	u64 buffer_address=_sanatize_user_memory((u64)user_buffer,packet.buffer_length);
	if (!buffer_address){
		regs->rax=0;
		return;
	}
	packet.buffer=VMM_TRANSLATE_ADDRESS(buffer_address);
	regs->rax=network_layer2_poll(&packet);
	packet.buffer=user_buffer;
	*((network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address))=packet;
}



static void _syscall_acpi_shutdown(syscall_registers_t* regs){
	acpi_fadt_shutdown(!!regs->rdi);
}



static void _syscall_memory_map(syscall_registers_t* regs){
	regs->rax=mmap_alloc(regs->rdi);
}



static void _syscall_memory_unmap(syscall_registers_t* regs){
	regs->rax=mmap_dealloc(regs->rdi,regs->rsi);
}



static void _syscall_clock_get_converion(syscall_registers_t* regs){
	regs->rax=clock_conversion_factor;
	regs->rdx=clock_conversion_shift;
	regs->r8=clock_cpu_frequency;
}



static void _syscall_drive_format(syscall_registers_t* regs){
	const drive_t* drive=drive_list_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	u64 address=0;
	if (regs->rdx){
		address=_sanatize_user_memory(regs->rsi,regs->rdx);
		if (!address){
			regs->rax=0;
			return;
		}
		address=(u64)VMM_TRANSLATE_ADDRESS(address);
	}
	regs->rax=kfs_format_drive(drive,(void*)address,regs->rdx);
}



static void _syscall_drive_stats(syscall_registers_t* regs){
	const drive_t* drive=drive_list_get_drive(regs->rdi);
	if (!drive||regs->rdx!=sizeof(drive_stats_t)){
		regs->rax=0;
		return;
	}
	u64 address=_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=0;
		return;
	}
	fs_flush_cache();
	*((drive_stats_t*)VMM_TRANSLATE_ADDRESS(address))=*(drive->stats);
}



static void _syscall_invalid(syscall_registers_t* regs,u64 number){
	ERROR("Invalid SYSCALL number: %lu",number);
	for (;;);
}



void syscall_init(void){
	LOG("Initializing SYSCALL table...");
	_syscall_handlers[0]=_syscall_serial_send;
	_syscall_handlers[1]=_syscall_serial_recv;
	_syscall_handlers[2]=_syscall_elf_load;
	_syscall_handlers[3]=_syscall_cpu_core_count;
	_syscall_handlers[4]=_syscall_cpu_core_start;
	_syscall_handlers[5]=_syscall_cpu_core_stop;
	_syscall_handlers[6]=_syscall_drive_list_length;
	_syscall_handlers[7]=_syscall_drive_list_get;
	_syscall_handlers[8]=_syscall_file_system_count;
	_syscall_handlers[9]=_syscall_file_system_get;
	_syscall_handlers[10]=_syscall_fd_open;
	_syscall_handlers[11]=_syscall_fd_close;
	_syscall_handlers[12]=_syscall_fd_delete;
	_syscall_handlers[13]=_syscall_fd_read;
	_syscall_handlers[14]=_syscall_fd_write;
	_syscall_handlers[15]=_syscall_fd_seek;
	_syscall_handlers[16]=_syscall_fd_stat;
	_syscall_handlers[17]=_syscall_fd_get_relative;
	_syscall_handlers[18]=_syscall_fd_move;
	_syscall_handlers[19]=_syscall_net_send;
	_syscall_handlers[20]=_syscall_net_poll;
	_syscall_handlers[21]=_syscall_acpi_shutdown;
	_syscall_handlers[22]=_syscall_memory_map;
	_syscall_handlers[23]=_syscall_memory_unmap;
	_syscall_handlers[24]=_syscall_clock_get_converion;
	_syscall_handlers[25]=_syscall_drive_format;
	_syscall_handlers[26]=_syscall_drive_stats;
	_syscall_handlers[SYSCALL_COUNT]=_syscall_invalid;
}
