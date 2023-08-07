#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/cpu.h>
#include <user/network.h>



static void _network_thread(void){
	u8 buffer[64];
	network_packet_t packet;
	while (1){
		packet.buffer_length=64;
		packet.buffer=buffer;
		network_poll(&packet);
	}
}



void main(void){
	cpu_core_start((cpu_bsp_id?0:1),_network_thread,NULL);
	cwd_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
