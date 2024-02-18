#include <coverage/coverage.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/util/util.h>
#include <test/acl.h>
#include <test/amm.h>
#include <test/cpu.h>
#include <test/crash.h>
#include <test/drive.h>
#include <test/elf.h>
#include <test/fd.h>
#include <test/format.h>
#include <test/fs.h>
#include <test/gid.h>
#include <test/handle_list.h>
#include <test/module.h>
#include <test/notification.h>
#include <test/pipe.h>
#include <test/resource.h>
#include <test/test.h>
#include <test/uid.h>
#define KERNEL_LOG_NAME "test"



static KERNEL_NOCOVERAGE _Bool _init(module_t* module){
	if (!coverage_init()){
		return 0;
	}
	test_acl();
	test_amm();
	test_cpu();
	test_crash();
	test_drive();
	test_elf();
	test_fd();
	test_format();
	test_fs();
	test_gid();
	test_handle_list();
	test_module();
	test_notification();
	test_pipe();
	test_resource();
	test_uid();
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
