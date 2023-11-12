#include <syscall/syscall.h>



void main(void){
	_syscall_fd_write(_syscall_fd_open(0,"/proc/self/stdout",17,2),"Hello (test lib loaded)!\n",8,0);
	for (;;);
}
