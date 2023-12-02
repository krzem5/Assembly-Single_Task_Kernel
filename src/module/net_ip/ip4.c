#include <kernel/log/log.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "net_ip4"



#define ETHER_TYPE 0x0800



static void _rx_callback(const network_layer1_packet_t* packet){
	WARN("PACKET (IPv4)");
}



static network_layer2_protocol_descriptor_t _net_ip4_network_layer2_protocol_descriptor={
	"IPv4",
	ETHER_TYPE,
	_rx_callback
};



void net_ip4_register_protocol(void){
	LOG("Registering IPv4 protocol...");
	network_layer2_register_descriptor(&_net_ip4_network_layer2_protocol_descriptor);
}
