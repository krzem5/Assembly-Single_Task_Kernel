#ifndef _SYS_UTIL_SORT_H_
#define _SYS_UTIL_SORT_H_ 1
#include <sys/types.h>



void __attribute__((access(read_write,1))) sys_sort(void* data,u64 size,u64 count,bool (*switch_callback)(const void*,const void*));



#endif
