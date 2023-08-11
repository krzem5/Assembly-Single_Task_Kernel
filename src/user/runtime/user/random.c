#include <user/syscall.h>
#include <user/types.h>



void random_bytes(void* buffer,u64 size){
	_syscall_random_generate(buffer,size);
}
