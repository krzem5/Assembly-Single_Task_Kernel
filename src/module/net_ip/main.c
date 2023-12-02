#include <kernel/module/module.h>
#include <net/ip4.h>
#include <net/ip6.h>



static _Bool _init(module_t* module){
	net_ip4_init();
	net_ip6_init();
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
