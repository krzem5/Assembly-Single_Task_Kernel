#include <kernel/module/module.h>
#include <kernel/types.h>



extern void test_module_mark_deinit_triggered(void);



MODULE_DEINIT(){
	test_module_mark_deinit_triggered();
}



MODULE_DECLARE_NEW(MODULE_FLAG_PREVENT_LOADS);
