#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



KERNEL_PUBLIC _Bool KERNEL_NOCOVERAGE streq(const char* a,const char* b){
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



KERNEL_PUBLIC void KERNEL_NOCOVERAGE strcpy(char* dst,const char* src,u64 max_length){
	if (!max_length){
		return;
	}
	for (u64 i=0;i<max_length-1;i++){
		dst[i]=src[i];
		if (!src[i]){
			break;
		}
	}
	dst[max_length-1]=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE strcpy_lowercase(char* dst,const char* src,u64 max_length){
	if (!max_length){
		return;
	}
	for (u64 i=0;i<max_length-1;i++){
		dst[i]=convert_lowercase(src[i]);
		if (!src[i]){
			break;
		}
	}
	dst[max_length-1]=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE memcpy_trunc_spaces(char* dst,const char* src,u8 length){
	for (;length&&src[length-1]==32;length--);
	memcpy(dst,src,length);
	dst[length]=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE memcpy_bswap16_trunc_spaces(const u16* src,u8 length,char* dst){
	u16* dst16=(u16*)dst;
	for (u8 i=0;i<length;i++){
		dst16[i]=__builtin_bswap16(src[i]);
	}
	u8 i=length<<1;
	do{
		i--;
	} while (i&&dst[i-1]==32);
	dst[i]=0;
}



KERNEL_PUBLIC void panic(const char* error){
	log("\x1b[1m\x1b[1m\x1b[38;2;192;28;40mFatal error: %s\x1b[0m\n",error);
	io_port_out16(0x604,0x2000);
	for (;;);
}
