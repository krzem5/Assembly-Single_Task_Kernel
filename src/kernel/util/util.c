#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



_Bool __attribute__((weak)) _user_panic_handler(const char* error){
	return 0;
}



KERNEL_PUBLIC void* KERNEL_NOCOVERAGE KERNEL_NOOPT (memcpy)(void* dst,const void* src,u64 length){
	if (!length){
		return dst;
	}
	const u8* src_ptr=src;
	u8* dst_ptr=dst;
	if (length<16){
		for (u64 i=0;i<length;i++){
			dst_ptr[i]=src_ptr[i];
		}
		return dst;
	}
	u8 padding=(-((u64)dst_ptr))&7;
	if (padding){
		for (u64 i=0;i<padding;i++){
			dst_ptr[i]=src_ptr[i];
		}
		src_ptr+=padding;
		dst_ptr+=padding;
		length-=padding;
	}
	u64 i=0;
	for (;i+8<=length;i+=8){
		*((u64*)(dst_ptr+i))=*((u64*)(src_ptr+i));
	}
	for (;i<length;i++){
		dst_ptr[i]=src_ptr[i];
	}
	return dst;
}



KERNEL_PUBLIC void* KERNEL_NOCOVERAGE KERNEL_NOOPT (memset)(void* dst,u8 value,u64 length){
	u8* ptr=dst;
	for (u64 i=0;i<length;i++){
		ptr[i]=value;
	}
	return dst;
}



KERNEL_PUBLIC _Bool KERNEL_NOCOVERAGE streq(const char* a,const char* b){
	while (*a&&*a==*b){
		a++;
		b++;
	}
	return (!*a&&!*b);
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



KERNEL_PUBLIC char* KERNEL_NOCOVERAGE memcpy_lowercase(char* dst,const char* src,u64 length){
	for (u64 i=0;i<length;i++){
		dst[i]=convert_lowercase(src[i]);
	}
	return dst;
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
	_user_panic_handler(error);
	log("\x1b[1m\x1b[1m\x1b[38;2;192;28;40mFatal error: %s\x1b[0m\n",error);
	io_port_out16(0x604,0x2000);
	for (;;);
}
