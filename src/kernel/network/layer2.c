#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>



void network_layer2_init(void){
	LOG("Initializing layer2 network...");
}



void network_layer2_process_packet(u64 packet,u16 length){
	const u8* data=VMM_TRANSLATE_ADDRESS(packet);
	INFO("Packet received: [%u] %x:%x:%x:%x:%x:%x",length,data[0],data[1],data[2],data[3],data[4],data[5]);
}
