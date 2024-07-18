#include <kernel/module/module.h>
#include <kernel/types.h>



extern void test_module_mark_deinit_triggered(void);



MODULE_INIT(){
	module_unload(module_self);
}



MODULE_DEINIT(){
	test_module_mark_deinit_triggered();
}



MODULE_DECLARE(0);
