#include <cwd.h>
#include <input.h>
#include <user/io.h>



void main(void){
	cwd_init();
	while (1){
		input_get();
	}
}
