#include <coverage/coverage.h>
#include <coverage/test/acl.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/module/module.h>
#include <kernel/util/util.h>



static _Bool _init(module_t* module){
	if (!coverage_init()){
		return 0;
	}
	coverage_test_acl();
	if (IS_ERROR(elf_load("/bin/coverage_test",0,NULL,0,NULL,0))){
		panic("Unable to load coverage tests");
	}
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
