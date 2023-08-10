#include <user/network.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool network_config(network_config_t* config){
	return _syscall_network_layer1_config(config,sizeof(network_config_t));
}



_Bool network_send(const network_packet_t* packet){
	return _syscall_network_layer2_send(packet,sizeof(network_packet_t));
}



_Bool network_poll(network_packet_t* packet,_Bool block){
	return _syscall_network_layer2_poll(packet,sizeof(network_packet_t),block);
}



void network_refresh_device_list(void){
	_syscall_network_layer3_refresh();
}



u32 network_device_count(void){
	return _syscall_network_layer3_device_count();
}



_Bool network_device_get(u32 index,network_device_t* device){
	return _syscall_network_layer3_device_get(index,device,sizeof(network_device_t));
}



_Bool network_device_delete(const u8* address){
	return _syscall_network_layer3_device_delete(address,6);
}
