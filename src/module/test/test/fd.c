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



extern error_t syscall_fd_open();
extern error_t syscall_fd_close();
extern error_t syscall_fd_read();
extern error_t syscall_fd_write();
extern error_t syscall_fd_seek();
extern error_t syscall_fd_resize();
extern error_t syscall_fd_stat();
extern error_t syscall_fd_dup();
extern error_t syscall_fd_path();
extern error_t syscall_fd_iter_start();
extern error_t syscall_fd_iter_get();
extern error_t syscall_fd_iter_next();
extern error_t syscall_fd_iter_stop();



static void _thread(void){
	// ==> fd_from_node
	// fd_from_node: no flags
	// fd_from_node: read dir
	// fd_from_node: no read permissions
	// fd_from_node: write dir
	// fd_from_node: no write permissions
	// fd_from_node: appends moves offset to EOF
	// fd_from_node: delete on exit => UNIMPLEMENTED
	// ==> fd_get_node
	// fd_get_node: invalid handle
	// fd_get_node: correct handle
	// ==> syscall_fd_open
	// syscall_fd_open: invalid flags => ERROR_INVALID_ARGUMENT(2)
	// syscall_fd_open: empty path => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_open: path >4095 => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_open: invalid root handle => ERROR_INVALID_HANDLE
	// syscall_fd_open: create file => UNIMPLEMENTED
	// syscall_fd_open: create directory => UNIMPLEMENTED
	// syscall_fd_open: do not follow links, not found => ERROR_NOT_FOUND
	// syscall_fd_open: do not follow links, found => !IS_ERROR(...)
	// syscall_fd_open: not found => ERROR_NOT_FOUND
	// syscall_fd_open: correct args => !IS_ERROR(...)
	// ==> syscall_fd_close
	// syscall_fd_close: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_close: no FD_ACL_FLAG_CLOSE => ERROR_DENIED
	// syscall_fd_close: correct args => ERROR_OK => handle becomes invalid
	// ==> syscall_fd_read
	// syscall_fd_read: invalid buffer pointer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_read: invalid flags => ERROR_INVALID_ARGUMENT(3)
	// syscall_fd_read: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_read: no FD_ACL_FLAG_IO => ERROR_DENIED
	// syscall_fd_read: no FD_FLAG_READ => ERROR_UNSUPPORTED_OPERATION
	// syscall_fd_read: correct args => !IS_ERROR(...) => read data is correct => advance offset
	// syscall_fd_read: nonblocking => !IS_ERROR(...) => correct flag passed to FS
	// syscall_fd_read: peek => !IS_ERROR(...) => next read is same value => next read is different value
	// ==> syscall_fd_write
	// syscall_fd_write: invalid buffer pointer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_write: invalid flags => ERROR_INVALID_ARGUMENT(3)
	// syscall_fd_write: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_write: no FD_ACL_FLAG_IO => ERROR_DENIED
	// syscall_fd_write: no FD_FLAG_WRITE => ERROR_UNSUPPORTED_OPERATION
	// syscall_fd_write: correct args => !IS_ERROR(...) => written data is correct => advance offset
	// syscall_fd_write: nonblocking => !IS_ERROR(...) => correct flag passed to FS
	// ==> syscall_fd_seek
	// syscall_fd_seek: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_seek: no FD_ACL_FLAG_IO => ERROR_DENIED
	// syscall_fd_seek: invalid type => ERROR_INVALID_ARGUMENT(2)
	// syscall_fd_seek: FD_SEEK_SET => correct offset
	// syscall_fd_seek: FD_SEEK_ADD => correct offset
	// syscall_fd_seek: FD_SEEK_END => correct offset
	// ==> syscall_fd_resize
	// syscall_fd_resize: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_resize: no FD_ACL_FLAG_IO => ERROR_DENIED
	// syscall_fd_resize: correct args => correct offset
	// syscall_fd_resize: shrink to zero => adjust offset correctly
	// ==> syscall_fd_stat
	// syscall_fd_stat: invalid buffer_length => ERROR_INVALID_ARGUMENT(2)
	// syscall_fd_stat: invalid buffer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_stat: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_stat: no FD_ACL_FLAG_STAT => ERROR_DENIED
	// syscall_fd_stat: correct args => !IS_ERROR(...) => correct data returned
	// ==> syscall_fd_dup
	// syscall_fd_dup: UNIMPLEMENTED
	// ==> syscall_fd_path
	// syscall_fd_path: invalid buffer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_path: invalid buffer_length => ERROR_NO_SPACE
	// syscall_fd_path: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_path: no FD_ACL_FLAG_STAT => ERROR_DENIED
	// syscall_fd_path: no space => ERROR_NO_SPACE
	// syscall_fd_path: correct args => !IS_ERROR(...) => correct data returned
	// ==> syscall_fd_iter_start
	// syscall_fd_iter_start: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_iter_start: no FD_ACL_FLAG_STAT => ERROR_DENIED
	// syscall_fd_iter_start: no read permission => ERROR_DENIED
	// syscall_fd_iter_start: correct args, no children => ERROR_NO_DATA
	// syscall_fd_iter_start: correct args, children => handle
	// ==> syscall_fd_iter_get
	// syscall_fd_iter_get: invalid buffer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_iter_get: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_iter_get: no FD_ITERATOR_ACL_FLAG_ACCESS => ERROR_DENIED
	// syscall_fd_iter_get: correct args => !IS_ERROR(...) => correct return value
	// ==> syscall_fd_iter_next
	// syscall_fd_iter_next: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_iter_next: no FD_ITERATOR_ACL_FLAG_ACCESS => ERROR_DENIED
	// syscall_fd_iter_next: correct args, no next child => ERROR_NO_DATA
	// syscall_fd_iter_next: correct args, next child => same handle
	// ==> syscall_fd_iter_stop
	// syscall_fd_iter_stop: UNIMPLEMENTED
}



void test_fd(void){
	LOG("Executing fd tests...");
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-cpu-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
