#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/cpu.h>
#include <user/layer3.h>
#include <user/io.h>



static void _network_thread(void){
	while (1){
		layer3_poll();
	}
}



void main(void){
	layer3_init();
	cpu_core_start((cpu_bsp_id?0:1),_network_thread,NULL);
	cwd_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
