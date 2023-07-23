#ifndef _KERNEL_NETWORK_LAYER2_H_
#define _KERNEL_NETWORK_LAYER2_H_ 1
#include <kernel/types.h>



void network_layer2_init(void);



void network_layer2_process_packet(u64 packet,u16 length);



#endif
