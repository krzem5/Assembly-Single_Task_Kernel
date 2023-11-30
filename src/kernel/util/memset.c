#include <kernel/types.h>



KERNEL_PUBLIC void* KERNEL_NOCOVERAGE (memset)(void* dst,u8 value,u64 length){
	u8* ptr=dst;
	if (length<16){
		switch (length){
			case 15:
				ptr[14]=value;
			case 14:
				ptr[13]=value;
			case 13:
				ptr[12]=value;
			case 12:
				ptr[11]=value;
			case 11:
				ptr[10]=value;
			case 10:
				ptr[9]=value;
			case 9:
				ptr[8]=value;
			case 8:
				ptr[7]=value;
			case 7:
				ptr[6]=value;
			case 6:
				ptr[5]=value;
			case 5:
				ptr[4]=value;
			case 4:
				ptr[3]=value;
			case 3:
				ptr[2]=value;
			case 2:
				ptr[1]=value;
			case 1:
				ptr[0]=value;
			case 0:
		}
		return dst;
	}
	u8 padding=(-((u64)ptr))&7;
	if (padding){
		for (u64 i=0;i<padding;i++){
			ptr[i]=value;
		}
		ptr+=padding;
		length-=padding;
	}
	u64 extended_value=0x0101010101010101ull*value;
	u64 i=0;
	for (;i<(length&0xfffffffffffffff8);i+=8){
		*((u64*)(ptr+i))=extended_value;
	}
	ptr+=i;
	switch (length&7){
		case 7:
			ptr[6]=value;
		case 6:
			ptr[5]=value;
		case 5:
			ptr[4]=value;
		case 4:
			ptr[3]=value;
		case 3:
			ptr[2]=value;
		case 2:
			ptr[1]=value;
		case 1:
			ptr[0]=value;
		case 0:
	}
	return dst;
}
