#include <kernel/types.h>



KERNEL_PUBLIC void KERNEL_NOCOVERAGE (mem_copy)(void* dst,const void* src,u64 length){
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
			case 0:
		}
		return;
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
		case 0:
	}
}
