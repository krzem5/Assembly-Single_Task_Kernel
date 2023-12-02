#include <kernel/log/log.h>
#include <kernel/types.h>
#include <net/ip4.h>
#include <net/udp.h>
#define KERNEL_LOG_NAME "net_udp"



#define PROTOCOL_TYPE 17



static void _rx_callback(const net_ip4_packet_t* packet){
	WARN("PACKED (UDP)");
}



static net_ip4_protocol_descriptor_t _net_udp_ip4_protocol_descriptor={
	"UDP",
	PROTOCOL_TYPE,
	_rx_callback
};



void net_udp_register_protocol(void){
	LOG("Registering UDP protocol...");
	net_ip4_register_protocol_descriptor(&_net_udp_ip4_protocol_descriptor);
}



KERNEL_PUBLIC net_udp_packet_t* net_udp_create_packet(u16 length);



KERNEL_PUBLIC void net_udp_delete_packet(net_udp_packet_t* packet);



KERNEL_PUBLIC void net_udp_send_packet(net_udp_packet_t* packet);
