#include <kernel/types.h>



KERNEL_PUBLIC _Bool KERNEL_NOCOVERAGE str_equal(const char* a,const char* b){
	u64 padding=(-((u64)a))&7;
	if (padding){
		for (u64 i=0;i<padding;i++){
			if (a[i]!=b[i]){
				return 0;
			}
			if (!a[i]){
				return 1;
			}
		}
		a+=padding;
		b+=padding;
	}
	for (u64 i=0;1;i+=8){
		u64 va=*((const u64*)(a+i));
		u64 vb=*((const u64*)(b+i));
		u64 m=(va-0x101010101010101ull)&0x8080808080808080ull&(~va);
		if (m){
			return !((va^vb)&(((m&(-m))<<1)-1));
		}
		if (va!=vb){
			return 0;
		}
	}
}
