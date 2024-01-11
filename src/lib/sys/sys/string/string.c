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



SYS_PUBLIC s32 sys_string_compare(const char* a,const char* b){
	return _string_compare(a,b,0);
}



SYS_PUBLIC s32 sys_string_compare_up_to(const char* a,const char* b,u64 length){
	if (!length){
		return 0;
	}
	return _string_compare(a,b,0);
}
