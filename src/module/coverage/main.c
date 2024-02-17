#include <coverage/coverage.h>
#include <coverage/test/acl.h>
#include <coverage/test/test.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "test"



static _Bool _init(module_t* module){
	if (!coverage_init()){
		return 0;
	}
	coverage_test_acl();
	WARN("%u test%s passed, %u test%s failed",coverage_test_pass_count,(coverage_test_pass_count==1?"":"s"),coverage_test_fail_count,(coverage_test_fail_count==1?"":"s"));
	if (coverage_test_fail_count){
		coverage_mark_failure();
		return 1;
	}
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
