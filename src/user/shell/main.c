#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/clock.h>
#include <user/drive.h>
#include <user/io.h>
#include <user/partition.h>



void main(void){
	clock_init();
	drive_init();
	partition_init();
	cwd_init();
	while (1){
		input_get();
		command_execute(input);
	}
}
