#include <command.h>
#include <input.h>



void main(void){
	while (1){
		input_get();
		command_execute(input);
	}
}
