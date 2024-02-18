#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_fd_close();
extern error_t syscall_pipe_create();



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(&(THREAD_DATA->process->mmap),0,2*PAGE_SIZE,NULL,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL,0);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	TEST_FUNC("syscall_pipe_create");
	// empty path
	// path too long
	// already present
	// not found
	// named
	// unnamed
	(void)buffer;
	mmap_dealloc_region(&(THREAD_DATA->process->mmap),temp_mmap_region);
}



void test_pipe(void){
	TEST_MODULE("pipe");
	TEST_FUNC("pipe_create");
	// named
	// unnamed
	TEST_FUNC("_pipe_read");
	// empty buffer
	// empty nonblocking read
	// blocking read
	// empty blocking read
	TEST_FUNC("_pipe_write");
	// empty buffer
	// full nonblocking write
	// blocking write
	// full blocking write
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-pipe-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
