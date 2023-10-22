#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "i82540"



static _Bool _init(module_t* module){
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"i82540",
	_init,
	_deinit
);
