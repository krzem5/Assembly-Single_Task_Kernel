#include <ip/ip4.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	ip4_register_protocol();
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
