#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "nvme"



static _Bool _init(module_t* module){
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"nvme",
	_init,
	_deinit
);
