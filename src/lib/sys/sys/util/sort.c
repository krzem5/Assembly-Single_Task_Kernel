#include <sys/memory/memory.h>
#include <sys/types.h>
#include <sys/util/sort.h>



SYS_PUBLIC void sys_sort(void* data,u64 size,u64 count,bool (*switch_callback)(const void*,const void*)){
	if (count<=1){
		return;
	}
	count--;
	u64 i=0;
	for (u64 j=0;j<count;j++){
		if (switch_callback(data+size*j,data+size*count)){
			sys_memory_exchange(data+size*i,data+size*j,size);
			i++;
		}
	}
	sys_memory_exchange(data+size*i,data+size*count,size);
	if (i>1){
		sys_sort(data,size,i,switch_callback);
	}
	i++;
	if (i<count){
		sys_sort(data+size*i,size,count-i+1,switch_callback);
	}
}
