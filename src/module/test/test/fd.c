#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test_fd"



extern error_t syscall_fd_close();
extern error_t syscall_fd_dup();
extern error_t syscall_fd_iter_get();
extern error_t syscall_fd_iter_next();
extern error_t syscall_fd_iter_start();
extern error_t syscall_fd_iter_stop();
extern error_t syscall_fd_open();
extern error_t syscall_fd_path();
extern error_t syscall_fd_read();
extern error_t syscall_fd_resize();
extern error_t syscall_fd_seek();
extern error_t syscall_fd_stat();
extern error_t syscall_fd_write();



static void _thread(void){
	return;
}



void test_fd(void){
	LOG("Executing fd tests...");
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-cpu-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
