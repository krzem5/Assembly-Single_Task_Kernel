#include <user/syscall.h>
#include <user/types.h>



_Bool elf_load(const char* path){
	u32 length=0;
	while (path[length]){
		length++;
	}
	return _syscall_elf_load(path,length);
}
