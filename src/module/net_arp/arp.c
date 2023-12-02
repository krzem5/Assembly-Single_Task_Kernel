#include <kernel/log/log.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <net/arp.h>
#include <net/ip4.h>
#define KERNEL_LOG_NAME "net_arp"



#define ETHER_TYPE 0x0806

#define CURRENT_IP_ADDRESS 0x0a00020f



static void _rx_callback(const network_layer1_packet_t* packet){
	if (packet->length<sizeof(net_arp_packet_t)){
		return;
	}
	const net_arp_packet_t* arp_packet=(const net_arp_packet_t*)(packet->data);
	if (arp_packet->oper!=__builtin_bswap16(NET_ARP_OPER_REPLY)||arp_packet->tpa!=__builtin_bswap32(CURRENT_IP_ADDRESS)){
		return;
	}
	WARN("PACKET (ARP) %x -> %X:%X:%X:%X:%X:%X",__builtin_bswap32(arp_packet->spa),arp_packet->sha[0],arp_packet->sha[1],arp_packet->sha[2],arp_packet->sha[3],arp_packet->sha[4],arp_packet->sha[5]);
}



static network_layer2_protocol_descriptor_t _net_arp_protocol_descriptor={
	"ARP",
	ETHER_TYPE,
	_rx_callback
};



void net_arp_init(void){
	LOG("Registering ARP protocol...");
	network_layer2_register_descriptor(&_net_arp_protocol_descriptor);
}



KERNEL_PUBLIC _Bool net_arp_resolve_address(net_ip4_address_t address,mac_address_t* out){
	if (!address){
		memset(out,0,sizeof(mac_address_t));
		return 1;
	}
	if (address==0xffffffff){
		memset(out,0xff,sizeof(mac_address_t));
		return 1;
	}
	if (!network_layer1_device){
		return 0;
	}
	mac_address_t dst_mac_address={0xff,0xff,0xff,0xff,0xff,0xff};
	network_layer1_packet_t* packet=network_layer1_create_packet(sizeof(net_arp_packet_t),&dst_mac_address,&(network_layer1_device->mac_address),ETHER_TYPE);
	net_arp_packet_t* arp_packet=(net_arp_packet_t*)(packet->data);
	arp_packet->htype=__builtin_bswap16(NET_ARP_HTYPE_ETHERNET);
	arp_packet->ptype=__builtin_bswap16(NET_ARP_PTYPE_IPV4);
	arp_packet->hlen=sizeof(mac_address_t);
	arp_packet->plen=sizeof(net_ip4_address_t);
	arp_packet->oper=__builtin_bswap16(NET_ARP_OPER_REQUEST);
	memcpy(arp_packet->sha,network_layer1_device->mac_address,sizeof(mac_address_t));
	arp_packet->spa=__builtin_bswap32(CURRENT_IP_ADDRESS);
	memset(arp_packet->tha,0,sizeof(mac_address_t));
	arp_packet->tpa=__builtin_bswap32(address);
	network_layer1_send_packet(packet);
	return 0;
}
