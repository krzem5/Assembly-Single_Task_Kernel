#include <command.h>
#include <cwd.h>
#include <input.h>



void main(void){
	cwd_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
