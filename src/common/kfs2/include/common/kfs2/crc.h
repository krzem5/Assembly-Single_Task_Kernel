#ifndef _COMMON_KFS2_CRC_H_
#define _COMMON_KFS2_CRC_H_ 1
#include <common/kfs2/util.h>



u32 kfs2_calculate_crc(const void* data,u64 size);



static inline __attribute__((always_inline)) void kfs2_insert_crc(void* data,u32 size){
	*((u32*)(data+size-4))=kfs2_calculate_crc(data,size-4);
}



static inline __attribute__((always_inline)) _Bool kfs2_verify_crc(const void* data,u32 size){
	return kfs2_calculate_crc(data,size-4)==*((const u32*)(data+size-4));
}



#endif
