#include <kernel/module/module.h>
#include <virtio/net.h>



static _Bool _init(module_t* module){
	virtio_net_init();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
