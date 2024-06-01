#include <coverage/coverage.h>
#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/mp/process.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
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
#include <test/module.h>
#include <test/pipe.h>
#include <test/resource.h>
#include <test/ring.h>
#include <test/test.h>
#include <test/uid.h>
#define KERNEL_LOG_NAME "test"



MODULE_INIT(){
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
	test_module();
	test_pipe();
	test_resource();
	test_ring();
	test_uid();
	WARN("%u test%s passed, %u test%s failed",test_pass_count,(test_pass_count==1?"":"s"),test_fail_count,(test_fail_count==1?"":"s"));
	if (test_fail_count){
		coverage_mark_failure();
		return;
	}
	error_t handle_id=elf_load("/bin/test",0,NULL,0,NULL,ELF_LOAD_FLAG_PAUSE_THREAD);
	if (IS_ERROR(handle_id)){
		goto _error;
	}
	handle_t* handle=handle_lookup_and_acquire(handle_id,process_handle_type);
	if (!handle){
		goto _error;
	}
	process_t* process=KERNEL_CONTAINEROF(handle,process_t,handle);
	process->vfs_stdin=vfs_lookup(NULL,"/dev/ser/in",VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0);
	process->vfs_stdout=vfs_lookup(NULL,"/dev/ser/out",VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0);
	process->vfs_stderr=vfs_lookup(NULL,"/dev/ser/out",VFS_LOOKUP_FLAG_FOLLOW_LINKS,0,0);
	scheduler_enqueue_thread(process->thread_list.head);
	handle_release(handle);
	return;
_error:
	panic("Unable to load test program");
	return;
}



MODULE_DECLARE(0);
