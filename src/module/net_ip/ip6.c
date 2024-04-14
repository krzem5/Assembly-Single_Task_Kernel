#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "net_ip6"



#define ETHER_TYPE 0x86dd



static void _rx_callback(network_layer1_packet_t* packet){
	WARN("PACKET (IPv6)");
}



static const network_layer2_protocol_descriptor_t _net_ip6_protocol_descriptor={
	"IPv6",
	ETHER_TYPE,
	_rx_callback
};



MODULE_POSTINIT(){
	LOG("Registering IPv6 protocol...");
	network_layer2_register_descriptor(&_net_ip6_protocol_descriptor);
}
