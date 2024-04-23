#include <kernel/types.h>



KERNEL_PUBLIC bool KERNEL_NOCOVERAGE str_startswith(const char* str,const char* prefix){
	while (prefix[0]){
		if (str[0]!=prefix[0]){
			return 0;
		}
		str++;
		prefix++;
	}
	return 1;
}
