#include <user/types.h>



_Bool string_equal(const char* a,const char* b){
	for (u32 i=0;a[i]||b[i];i++){
		if (a[i]!=b[i]){
			return 0;
		}
	}
	return 1;
}
