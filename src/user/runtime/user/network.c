#include <user/network.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool network_get_mac_address(u8* mac_address){
	return _syscall_network_layer1_get_mac_address(mac_address,6);
}



_Bool network_send(const network_packet_t* packet){
	return _syscall_network_layer2_send(packet,sizeof(network_packet_t));
}



_Bool network_poll(network_packet_t* packet,_Bool block){
	return _syscall_network_layer2_poll(packet,sizeof(network_packet_t),block);
}
