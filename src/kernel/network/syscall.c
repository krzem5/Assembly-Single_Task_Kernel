#include <kernel/isr/isr.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_network_layer1_get_mac_address(isr_state_t* regs){
	if (regs->rsi!=6||!syscall_sanatize_user_memory(regs->rdi,regs->rsi)){
		regs->rax=0;
		return;
	}
	memcpy((void*)(regs->rdi),network_layer1_mac_address,6);
	regs->rax=1;
}



void syscall_network_layer2_send(isr_state_t* regs){
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



void syscall_network_layer2_poll(isr_state_t* regs){
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
