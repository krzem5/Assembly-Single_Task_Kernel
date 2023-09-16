#include <kernel/memory/vmm.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_network_layer2_send(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(network_layer2_packet_t)||!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	network_layer2_packet_t packet=*((const network_layer2_packet_t*)(regs->rdi));
	u64 buffer_address=syscall_sanatize_user_memory((u64)(packet.buffer),packet.buffer_length);
	if (!buffer_address){
		regs->rax=0;
		return;
	}
	packet.buffer=(void*)buffer_address;
	regs->rax=network_layer2_send(&packet);
}



void syscall_network_layer2_poll(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(network_layer2_packet_t)||!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	network_layer2_packet_t* packet=(network_layer2_packet_t*)(regs->rdi);
	if (!syscall_sanatize_user_memory((u64)(packet->buffer),packet->buffer_length)){
		regs->rax=0;
		return;
	}
	regs->rax=network_layer2_poll(packet,!!regs->rdx);
}



void syscall_network_layer3_refresh(syscall_registers_t* regs){
	network_layer3_refresh_device_list();
}



void syscall_network_layer3_device_count(syscall_registers_t* regs){
	regs->rax=network_layer3_get_device_count();
}



void syscall_network_layer3_device_get(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(network_layer3_device_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	const network_layer3_device_t* device=network_layer3_get_device(regs->rdi);
	if (!device){
		regs->rax=0;
		return;
	}
	*((network_layer3_device_t*)(regs->rsi))=*device;
	regs->rax=1;
}



void syscall_network_layer3_device_delete(syscall_registers_t* regs){
	if (regs->rsi!=6||!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	regs->rax=network_layer3_delete_device((void*)(regs->rdi));
}
