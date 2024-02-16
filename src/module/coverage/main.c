#include <coverage/coverage.h>
#include <coverage/test/acl.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	coverage_init();
	coverage_test_acl();
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
