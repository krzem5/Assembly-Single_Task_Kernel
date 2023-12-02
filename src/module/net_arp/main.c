#include <kernel/module/module.h>
#include <net/arp.h>



static _Bool _init(module_t* module){
	net_arp_register_protocol();
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
