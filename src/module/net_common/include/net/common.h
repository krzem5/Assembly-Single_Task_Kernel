#ifndef _NET_COMMON_H_
#define _NET_COMMON_H_ 1
#include <kernel/types.h>



void net_common_calculate_checksum(const void* data,u16 length,u16* out);



void net_common_update_checksum(const void* data,u16 length,u16* out);



_Bool net_common_verify_checksum(const void* data,u16 length);



#endif
