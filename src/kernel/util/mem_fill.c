#include <kernel/types.h>



KERNEL_PUBLIC void KERNEL_NOCOVERAGE (mem_fill)(void* ptr,u64 length,u8 value){
	u8* ptr8=ptr;
	if (length<16){
		switch (length){
			case 15:
				ptr8[14]=value;
			case 14:
				ptr8[13]=value;
			case 13:
				ptr8[12]=value;
			case 12:
				ptr8[11]=value;
			case 11:
				ptr8[10]=value;
			case 10:
				ptr8[9]=value;
			case 9:
				ptr8[8]=value;
			case 8:
				ptr8[7]=value;
			case 7:
				ptr8[6]=value;
			case 6:
				ptr8[5]=value;
			case 5:
				ptr8[4]=value;
			case 4:
				ptr8[3]=value;
			case 3:
				ptr8[2]=value;
			case 2:
				ptr8[1]=value;
			case 1:
				ptr8[0]=value;
			case 0:
		}
		return;
	}
	u8 padding=(-((u64)ptr8))&7;
	if (padding){
		for (u64 i=0;i<padding;i++){
			ptr8[i]=value;
		}
		ptr8+=padding;
		length-=padding;
	}
	u64 extended_value=0x0101010101010101ull*value;
	u64 i=0;
	for (;i<(length&0xfffffffffffffff8);i+=8){
		*((u64*)(ptr8+i))=extended_value;
	}
	ptr8+=i;
	switch (length&7){
		case 7:
			ptr8[6]=value;
		case 6:
			ptr8[5]=value;
		case 5:
			ptr8[4]=value;
		case 4:
			ptr8[3]=value;
		case 3:
			ptr8[2]=value;
		case 2:
			ptr8[1]=value;
		case 1:
			ptr8[0]=value;
		case 0:
	}
}
