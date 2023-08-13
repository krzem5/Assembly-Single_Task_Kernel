#include <kernel/memory/vmm.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/network/layer3.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



typedef struct _USER_NETWORK_CONFIG{
	char name[16];
	u8 address[6];
} user_network_config_t;



void syscall_network_layer1_config(syscall_registers_t* regs){
	if (regs->rsi!=sizeof(user_network_config_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	user_network_config_t* config=VMM_TRANSLATE_ADDRESS(address);
	u8 i=0;
	if (network_layer1_name){
		for (;network_layer1_name[i];i++){
			config->name[i]=network_layer1_name[i];
		}
	}
	config->name[i]=0;
	for (i=0;i<6;i++){
		config->address[i]=network_layer1_mac_address[i];
	}
}



void syscall_network_layer2_send(syscall_registers_t* regs){
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



void syscall_network_layer2_poll(syscall_registers_t* regs){
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
	regs->rax=network_layer2_poll(&packet,!!regs->rdx);
	packet.buffer=user_buffer;
	*((network_layer2_packet_t*)VMM_TRANSLATE_ADDRESS(address))=packet;
}



void syscall_network_layer3_refresh(syscall_registers_t* regs){
	network_layer3_refresh_device_list();
}



void syscall_network_layer3_device_count(syscall_registers_t* regs){
	regs->rax=network_layer3_get_device_count();
}



void syscall_network_layer3_device_get(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(network_layer3_device_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=0;
		return;
	}
	const network_layer3_device_t* device=network_layer3_get_device(regs->rdi);
	if (!device){
		regs->rax=0;
		return;
	}
	*((network_layer3_device_t*)VMM_TRANSLATE_ADDRESS(address))=*device;
	regs->rax=1;
}



void syscall_network_layer3_device_delete(syscall_registers_t* regs){
	if (regs->rsi!=6){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rdi,regs->rsi);
	if (!address){
		regs->rax=0;
		return;
	}
	regs->rax=network_layer3_delete_device(VMM_TRANSLATE_ADDRESS(address));
}
