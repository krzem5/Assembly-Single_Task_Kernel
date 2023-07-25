#include <kernel/log/log.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>



static network_layer1_device_t _network_layer1_device;

const char* network_layer1_name;
mac_address_t network_layer1_mac_address;



void network_layer1_init(void){
	LOG("Initializing layer1 network...");
	_network_layer1_device.name=NULL;
	network_layer1_name=NULL;
	mac_address_init(&network_layer1_mac_address);
}



void network_layer1_set_device(const network_layer1_device_t* device){
	if (_network_layer1_device.name){
		WARN("Layer1 network device already installed");
		return;
	}
	LOG("Enabling layer1 network device '%s'...",device->name);
	_network_layer1_device=*device;
	network_layer1_name=_network_layer1_device.name;
	for (u8 i=0;i<6;i++){
		network_layer1_mac_address[i]=_network_layer1_device.mac_address[i];
	}
	INFO("Layer1 network MAC address: %x:%x:%x:%x:%x:%x",_network_layer1_device.mac_address[0],_network_layer1_device.mac_address[1],_network_layer1_device.mac_address[2],_network_layer1_device.mac_address[3],_network_layer1_device.mac_address[4],_network_layer1_device.mac_address[5]);
}



void network_layer1_send(u64 packet,u16 length){
	if (!_network_layer1_device.name){
		return;
	}
	_network_layer1_device.tx(_network_layer1_device.extra_data,packet,length);
}



u16 network_layer1_poll(void* buffer,u16 buffer_length){
	if (!_network_layer1_device.name){
		return 0;
	}
	return _network_layer1_device.rx(_network_layer1_device.extra_data,buffer,buffer_length);
}
