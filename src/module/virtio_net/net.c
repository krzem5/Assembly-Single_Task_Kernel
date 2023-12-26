#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/device.h>
#include <virtio/net.h>
#define KERNEL_LOG_NAME "virtio_net"



static const virtio_device_driver_t _virtio_net_device_driver={
	"Network Device",
	0x0001,
	0
};



void virtio_net_init(void){
	LOG("Initializing VirtIO network driver...");
	if (!virtio_register_device_driver(&_virtio_net_device_driver)){
		ERROR("Unable to register VirtIO network driver");
	}
}
