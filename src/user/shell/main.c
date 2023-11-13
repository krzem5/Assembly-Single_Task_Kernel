#include <command.h>
#include <cwd.h>
#include <input.h>
#include <user/clock.h>
#include <user/io.h>



extern u64 CORE_TEST_FUNC(void);



void main(void){
	cwd_init();
	printf("CORE_TEST_FUNC = %p [%u]\n",CORE_TEST_FUNC,CORE_TEST_FUNC());
	while (1){
		input_get();
		command_execute(input);
	}
}
