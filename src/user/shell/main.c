#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/clock.h>



void main(void){
	cwd_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
