#include <kernel/log/log.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "net_arp"



#define ETHER_TYPE 0x0806



static void _rx_callback(const network_layer1_packet_t* packet){
	WARN("PACKET (ARP)");
}



static network_layer2_protocol_descriptor_t _net_arp_network_layer2_protocol_descriptor={
	"ARP",
	ETHER_TYPE,
	_rx_callback
};



void net_arp_register_protocol(void){
	LOG("Registering ARP protocol...");
	network_layer2_register_descriptor(&_net_arp_network_layer2_protocol_descriptor);
}
