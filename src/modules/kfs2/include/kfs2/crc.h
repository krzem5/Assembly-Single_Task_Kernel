#ifndef _KFS2_CRC_H_
#define _KFS2_CRC_H_ 1
#include <kernel/types.h>



u32 kfs2_calculate_crc(const void* data,u64 size);



static KERNEL_INLINE _Bool kfs2_verify_crc(const void* data,u32 size){
	return kfs2_calculate_crc(data,size-4)==*((const u32*)(data+size-4));
}



#endif
