#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



_Bool KERNEL_CORE_CODE __attribute__((weak)) _user_panic_handler(const char* error){
	return 0;
}



void* KERNEL_CORE_CODE KERNEL_NOOPT memcpy(void* dst,const void* src,u64 length){
	u8* dst_ptr=dst;
	const u8* src_ptr=src;
	for (u64 i=0;i<length;i++){
		dst_ptr[i]=src_ptr[i];
	}
	return dst;
}



void* KERNEL_CORE_CODE KERNEL_NOOPT memset(void* dst,u8 value,u64 length){
	u8* ptr=dst;
	for (u64 i=0;i<length;i++){
		ptr[i]=value;
	}
	return dst;
}



void KERNEL_CORE_CODE bswap16_trunc_spaces(const u16* src,u8 length,char* dst){
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



void KERNEL_CORE_CODE panic(const char* error,_Bool recoverable){
	if (_user_panic_handler(error)&&recoverable){
		return;
	}
	log("\x1b[1m\x1b[1m\x1b[38;2;192;28;40mFatal error: %u\x1b[0m\n",error);
	io_port_out16(0x604,0x2000);
	for (;;);
}

