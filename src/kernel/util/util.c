#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



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



KERNEL_PUBLIC void KERNEL_NOCOVERAGE memcpy_trunc_spaces(char* dst,const char* src,u64 length){
	for (;length&&src[length-1]==32;length--);
	memcpy(dst,src,length);
	dst[length]=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE memcpy_bswap16_trunc_spaces(const u16* src,u64 length,char* dst){
	u16* dst16=(u16*)dst;
	for (u64 i=0;i<length;i++){
		dst16[i]=__builtin_bswap16(src[i]);
	}
	u64 i=length<<1;
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
