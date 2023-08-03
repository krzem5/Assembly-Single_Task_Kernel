#include <kernel/memory/vmm.h>
#include <kernel/network/layer2.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_net_send(syscall_registers_t* regs){
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



void syscall_net_poll(syscall_registers_t* regs){
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
