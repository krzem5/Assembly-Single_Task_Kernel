#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/clock.h>



const char interp[] __attribute__((used,section(".interp")))="/lib/ld.elf";



void main(void){
	clock_init();
	cwd_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
