#include <command.h>
#include <input.h>



void main(void){
	while (1){
		command_execute(input_get());
	}
}
