#include <user/io.h>



char cwd[4096]="/";



static void _print_line_start(void){
	printf("\x1b[1m\x1b[38;2;67;154;6mshell\x1b[0m:\x1b[1m\x1b[38;2;52;101;164m%s\x1b[0m$ \n",cwd);
}



void main(void){
	printf("Starting shell...\n");
	_print_line_start();
}
