#include <coverage/coverage.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/util/util.h>
#include <test/acl.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static KERNEL_NOCOVERAGE _Bool _init(module_t* module){
	if (!coverage_init()){
		return 0;
	}
	test_acl();
	WARN("%u test%s passed, %u test%s failed",test_pass_count,(test_pass_count==1?"":"s"),test_fail_count,(test_fail_count==1?"":"s"));
	if (test_fail_count){
		coverage_mark_failure();
		return 1;
	}
	if (IS_ERROR(elf_load("/bin/test",0,NULL,0,NULL,0))){
		panic("Unable to load test program");
	}
	return 1;
}



static KERNEL_NOCOVERAGE void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
