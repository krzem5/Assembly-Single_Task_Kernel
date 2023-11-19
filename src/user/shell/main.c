#include <command.h>
#include <cwd.h>
#include <dircolor/dircolor.h>
#include <input.h>
#include <sys/clock.h>
#include <sys/io.h>



void main(void){
	cwd_init();
	dircolor_init();
	sys_clock_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
