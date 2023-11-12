#include <syscall/syscall.h>



#define AT_NULL 0
#define AT_IGNORE 1
#define AT_PHDR 3
#define AT_PHENT 4
#define AT_PHNUM 5
#define AT_PAGESZ 6
#define AT_BASE 7
#define AT_FLAGS 8
#define AT_ENTRY 9
#define AT_PLATFORM 15
#define AT_HWCAP 16
#define AT_RANDOM 25
#define AT_HWCAP2 26
#define AT_EXECFN 31



u64 main(const u64* data){
	for (data+=data[0]+1;data[0];data++);
	for (data++;data[0];data+=2){
		if (data[0]==AT_ENTRY){
			return data[1];
		}
	}
	_syscall_fd_write(_syscall_fd_open(0,"/proc/self/stdout",17,2),"Hello!\n",8,0);
	for (;;);
}
