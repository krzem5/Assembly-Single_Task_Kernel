#include <core/syscall.h>



void main(const void* data){
	_syscall_fd_write(_syscall_fd_open(0,"/proc/self/stdout",17,2),"Hello (test library loaded)!\n",8,0);
	for (;;);
}
