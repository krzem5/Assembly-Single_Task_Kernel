#include <user/syscall.h>
#include <user/types.h>



void elf_load(const char* path){
	u32 length=0;
	while (path[length]){
		length++;
	}
	_syscall_elf_load(path,length);
}
