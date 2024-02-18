#include <kernel/module/module.h>
#include <kernel/types.h>



extern void test_module_mark_deinit_triggered(void);



static _Bool _init(module_t* module){
	return 0;
}



static void _deinit(module_t* module){
	test_module_mark_deinit_triggered();
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);

