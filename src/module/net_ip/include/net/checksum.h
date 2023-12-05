#ifndef _NET_CHECKSUM_H_
#define _NET_CHECKSUM_H_ 1
#include <kernel/types.h>



void net_checksum_calculate_checksum(const void* data,u16 length,u16* out);



void net_checksum_update_checksum(const void* data,u16 length,u16* out);



u16 net_checksum_verify_checksum(const void* data,u16 length,u32 checksum);



#endif
