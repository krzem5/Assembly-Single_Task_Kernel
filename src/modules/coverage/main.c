#include <coverage/coverage.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	coverage_export();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"coverage",
	_init,
	_deinit
);
