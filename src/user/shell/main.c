#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/clock.h>
#include <user/io.h>
#include <user/network.h>
#include <user/thread.h>



static void _network_thread(void* arg){
	u8 buffer[64];
	network_packet_t packet;
	while (1){
		packet.buffer_length=64;
		packet.buffer=buffer;
		network_poll(&packet,1);
	}
}



void main(void){
	clock_init();
	cwd_init();
	u64 tid=thread_create(_network_thread,NULL,0);
	thread_set_priority(tid,THREAD_PRIORITY_LOW);
	while (1){
		input_get();
		command_execute(input);
	}
}
