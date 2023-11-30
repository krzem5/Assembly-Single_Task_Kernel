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
		switch (length){
			case 15:
				dst_ptr[14]=src_ptr[14];
			case 14:
				dst_ptr[13]=src_ptr[13];
			case 13:
				dst_ptr[12]=src_ptr[12];
			case 12:
				dst_ptr[11]=src_ptr[11];
			case 11:
				dst_ptr[10]=src_ptr[10];
			case 10:
				dst_ptr[9]=src_ptr[9];
			case 9:
				dst_ptr[8]=src_ptr[8];
			case 8:
				dst_ptr[7]=src_ptr[7];
			case 7:
				dst_ptr[6]=src_ptr[6];
			case 6:
				dst_ptr[5]=src_ptr[5];
			case 5:
				dst_ptr[4]=src_ptr[4];
			case 4:
				dst_ptr[3]=src_ptr[3];
			case 3:
				dst_ptr[2]=src_ptr[2];
			case 2:
				dst_ptr[1]=src_ptr[1];
			case 1:
				dst_ptr[0]=src_ptr[0];
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
	for (;i<(length&0xfffffffffffffff8);i+=8){
		*((u64*)(dst_ptr+i))=*((u64*)(src_ptr+i));
	}
	src_ptr+=i;
	dst_ptr+=i;
	switch (length&7){
		case 7:
			dst_ptr[6]=src_ptr[6];
		case 6:
			dst_ptr[5]=src_ptr[5];
		case 5:
			dst_ptr[4]=src_ptr[4];
		case 4:
			dst_ptr[3]=src_ptr[3];
		case 3:
			dst_ptr[2]=src_ptr[2];
		case 2:
			dst_ptr[1]=src_ptr[1];
		case 1:
			dst_ptr[0]=src_ptr[0];
	}
	return dst;
}



KERNEL_PUBLIC void* KERNEL_NOCOVERAGE KERNEL_NOOPT (memset)(void* dst,u8 value,u64 length){
	if (!length){
		return dst;
	}
	u8* ptr=dst;
	if (length<16){
		for (u64 i=0;i<length;i++){
			ptr[i]=value;
		}
		return dst;
	}
	u8 padding=(-((u64)ptr))&7;
	if (padding){
		for (u64 i=0;i<padding;i++){
			ptr[i]=0;
		}
		ptr+=padding;
		length-=padding;
	}
	u64 extended_value=0x0101010101010101ull*value;
	u64 i=0;
	for (;i+8<=length;i+=8){
		*((u64*)(ptr+i))=extended_value;
	}
	for (;i<length;i++){
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
