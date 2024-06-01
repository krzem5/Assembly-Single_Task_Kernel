#include <sys/heap/heap.h>
#include <sys/memory/memory.h>
#include <sys/types.h>



static s32 _string_compare(const char* a,const char* b,u64 length){
	do{
		if (a[0]!=b[0]){
			return a[0]-b[0];
		}
		if (!a[0]){
			return 0;
		}
		a++;
		b++;
		length--;
	} while (length);
	return 0;
}



SYS_PUBLIC u64 sys_string_length(const char* str){
	u64 out=0;
	for (;str[out];out++);
	return out;
}



SYS_PUBLIC s32 sys_string_compare(const char* a,const char* b){
	return _string_compare(a,b,0);
}



SYS_PUBLIC s32 sys_string_compare_up_to(const char* a,const char* b,u64 length){
	if (!length){
		return 0;
	}
	return _string_compare(a,b,length);
}



SYS_PUBLIC char* sys_string_duplicate(const char* str){
	u64 length=sys_string_length(str);
	char* out=sys_heap_alloc(NULL,length);
	sys_memory_copy(str,out,length+1);
	return out;
}



SYS_PUBLIC void sys_string_copy(const char* src,char* dst){
	for (u64 i=0;1;i++){
		dst[i]=src[i];
		if (!src[i]){
			return;
		}
	}
}
