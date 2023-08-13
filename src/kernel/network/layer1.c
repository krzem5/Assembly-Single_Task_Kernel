#include <kernel/log/log.h>
#include <kernel/network/layer1.h>
#include <kernel/network/layer2.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "layer1"



static network_layer1_device_t KERNEL_CORE_BSS _layer1_device;

const char* KERNEL_CORE_BSS network_layer1_name;
mac_address_t KERNEL_CORE_BSS network_layer1_mac_address;



void network_layer1_init_irq(void){
	LOG("Initializing layer1 network device IRQ...");
	_layer1_device.irq_init(_layer1_device.extra_data);
}



void KERNEL_CORE_CODE network_layer1_set_device(const network_layer1_device_t* device){
	if (network_layer1_name){
		WARN_CORE("Layer1 network device already installed");
		return;
	}
	LOG_CORE("Enabling layer1 network device '%s'...",device->name);
	_layer1_device=*device;
	network_layer1_name=_layer1_device.name;
	for (u8 i=0;i<6;i++){
		network_layer1_mac_address[i]=_layer1_device.mac_address[i];
	}
	INFO_CORE("Layer1 network MAC address: %x:%x:%x:%x:%x:%x",_layer1_device.mac_address[0],_layer1_device.mac_address[1],_layer1_device.mac_address[2],_layer1_device.mac_address[3],_layer1_device.mac_address[4],_layer1_device.mac_address[5]);
}



void network_layer1_send(u64 packet,u16 length){
	if (!network_layer1_name){
		return;
	}
	_layer1_device.tx(_layer1_device.extra_data,packet,length);
}



u16 network_layer1_poll(void* buffer,u16 buffer_length){
	if (!network_layer1_name){
		return 0;
	}
	return _layer1_device.rx(_layer1_device.extra_data,buffer,buffer_length);
}



void network_layer1_wait(void){
	if (!network_layer1_name){
		return;
	}
	_layer1_device.wait(_layer1_device.extra_data);
}
