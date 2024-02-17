#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



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



static void _set_acl_flags(handle_id_t handle_id,u64 clear,u64 set){
	handle_t* handle=handle_lookup_and_acquire(handle_id,HANDLE_ID_GET_TYPE(handle_id));
	if (handle&&handle->acl){
		acl_set(handle->acl,THREAD_DATA->process,clear,set);
	}
	handle_release(handle);
}



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(&(THREAD_DATA->process->mmap),0,2*PAGE_SIZE,NULL,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL,0);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	vfs_node_t* root=vfs_lookup(NULL,"/",0,0,0);
	TEST_FUNC("fd_from_node");
	TEST_GROUP("no flags");
	error_t fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("read directory");
	fd=fd_from_node(root,FD_FLAG_READ);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_read(fd,buffer,PAGE_SIZE,0)==ERROR_UNSUPPORTED_OPERATION);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("no read permissions");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/no_read_access_file",0,0,0),FD_FLAG_READ);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_read(fd,buffer,PAGE_SIZE,0)==ERROR_UNSUPPORTED_OPERATION);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("write directory");
	fd=fd_from_node(root,FD_FLAG_WRITE);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_write(fd,buffer,PAGE_SIZE,0)==ERROR_UNSUPPORTED_OPERATION);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("no write permissions");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/no_write_access_file",0,0,0),FD_FLAG_WRITE);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_write(fd,buffer,PAGE_SIZE,0)==ERROR_UNSUPPORTED_OPERATION);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("correct args");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/length_6_file",0,0,0),FD_FLAG_READ);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(!syscall_fd_seek(fd,0,FD_SEEK_ADD));
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("appends moves offset to EOF");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/length_6_file",0,0,0),FD_FLAG_READ|FD_FLAG_APPEND);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_seek(fd,0,FD_SEEK_ADD)==6);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	// fd_from_node: delete on exit => UNIMPLEMENTED
	TEST_FUNC("fd_get_node");
	TEST_GROUP("invalid handle");
	TEST_ASSERT(!fd_get_node(0xaabbccdd));
	TEST_GROUP("correct handle");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(fd_get_node(fd)==root);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_FUNC("syscall_fd_open");
	TEST_GROUP("invalid flags");
	TEST_ASSERT(syscall_fd_open(0,NULL,0xffffffffffffffff)==ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("empty path");
	buffer[0]=0;
	TEST_ASSERT(syscall_fd_open(0,buffer,0)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("path too long");
	memset(buffer,'A',2*PAGE_SIZE);
	TEST_ASSERT(syscall_fd_open(0,buffer,0)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid root handle");
	strcpy(buffer,"/",PAGE_SIZE);
	TEST_ASSERT(syscall_fd_open(0xaabbccdd,buffer,0)==ERROR_INVALID_HANDLE);
	// syscall_fd_open: create file => UNIMPLEMENTED
	// syscall_fd_open: create directory => UNIMPLEMENTED
	// syscall_fd_open: do not follow links, not found => ERROR_NOT_FOUND
	// syscall_fd_open: do not follow links, found => !IS_ERROR(...)
	TEST_GROUP("not found");
	strcpy(buffer,"/invalid/path",PAGE_SIZE);
	TEST_ASSERT(syscall_fd_open(0,buffer,0)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	strcpy(buffer,"/",PAGE_SIZE);
	fd=syscall_fd_open(0,buffer,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(fd_get_node(fd)==root);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	// syscall_fd_open: parent, not found => ERROR_NOT_FOUND
	// syscall_fd_open: parent, found => !IS_ERROR(...)
	TEST_FUNC("syscall_fd_close");
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_fd_close(0xaabbccdd)==ERROR_INVALID_HANDLE);
	TEST_GROUP("no FD_ACL_FLAG_CLOSE");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	_set_acl_flags(fd,FD_ACL_FLAG_CLOSE,0);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_DENIED);
	_set_acl_flags(fd,0,FD_ACL_FLAG_CLOSE);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("correct args");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_ASSERT(!fd_get_node(fd));
	TEST_FUNC("syscall_fd_read");
	TEST_GROUP("invalid buffer pointer");
	TEST_ASSERT(syscall_fd_read(0,NULL,1,0)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid flags");
	TEST_ASSERT(syscall_fd_read(0,buffer,0,0xffffffffffffffff)==ERROR_INVALID_ARGUMENT(3));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_fd_read(0xaabbccdd,buffer,0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("no FD_ACL_FLAG_IO");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	_set_acl_flags(fd,FD_ACL_FLAG_IO,0);
	TEST_ASSERT(syscall_fd_read(fd,buffer,0,0)==ERROR_DENIED);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("no FD_FLAG_READ");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/length_6_file",0,0,0),0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_read(fd,buffer,PAGE_SIZE,0)==ERROR_UNSUPPORTED_OPERATION);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("correct args");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/length_6_file",0,0,0),FD_FLAG_READ);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_read(fd,buffer,PAGE_SIZE,0)==6);
	TEST_ASSERT(buffer[0]=='a'&&buffer[1]=='b'&&buffer[2]=='c'&&buffer[3]=='d'&&buffer[4]=='e'&&buffer[5]=='f');
	TEST_ASSERT(syscall_fd_seek(fd,0,FD_SEEK_ADD)==6);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	// syscall_fd_read: nonblocking => !IS_ERROR(...) => correct flag passed to FS
	// syscall_fd_read: peek => !IS_ERROR(...) => next read is same value => next read is different value
	TEST_FUNC("syscall_fd_write");
	TEST_GROUP("invalid buffer pointer");
	TEST_ASSERT(syscall_fd_write(0,NULL,1,0)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid flags");
	TEST_ASSERT(syscall_fd_write(0,buffer,0,0xffffffffffffffff)==ERROR_INVALID_ARGUMENT(3));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_fd_write(0xaabbccdd,buffer,0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("no FD_ACL_FLAG_IO");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	_set_acl_flags(fd,FD_ACL_FLAG_IO,0);
	TEST_ASSERT(syscall_fd_write(fd,buffer,0,0)==ERROR_DENIED);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("no FD_FLAG_WRITE");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/length_6_file",0,0,0),0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_write(fd,buffer,PAGE_SIZE,0)==ERROR_UNSUPPORTED_OPERATION);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	// syscall_fd_write: correct args => !IS_ERROR(...) => UNIMPLEMENTED
	// syscall_fd_write: nonblocking => !IS_ERROR(...) => correct flag passed to FS
	TEST_FUNC("syscall_fd_seek");
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_fd_seek(0xaabbccdd,0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("no FD_ACL_FLAG_IO");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	_set_acl_flags(fd,FD_ACL_FLAG_IO,0);
	TEST_ASSERT(syscall_fd_seek(fd,0,0)==ERROR_DENIED);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("invalid type");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_seek(fd,0,0xaabbccdd)==ERROR_INVALID_ARGUMENT(2));
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("correct args, FD_SEEK_SET");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_seek(fd,3,FD_SEEK_SET)==3);
	TEST_ASSERT(syscall_fd_seek(fd,2,FD_SEEK_SET)==2);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("correct args, FD_SEEK_ADD");
	fd=fd_from_node(root,0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_seek(fd,2,FD_SEEK_ADD)==2);
	TEST_ASSERT(syscall_fd_seek(fd,(s64)(-1),FD_SEEK_ADD)==1);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_GROUP("correct args, FD_SEEK_END");
	fd=fd_from_node(vfs_lookup(NULL,"/share/test/fd/length_6_file",0,0,0),0);
	TEST_ASSERT(!IS_ERROR(fd));
	TEST_ASSERT(syscall_fd_seek(fd,0,FD_SEEK_END)==6);
	TEST_ASSERT(syscall_fd_seek(fd,2,FD_SEEK_SET)==2);
	TEST_ASSERT(syscall_fd_seek(fd,1,FD_SEEK_END)==5);
	TEST_ASSERT(syscall_fd_close(fd)==ERROR_OK);
	TEST_FUNC("syscall_fd_resize");
	// syscall_fd_resize: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_resize: no FD_ACL_FLAG_IO => ERROR_DENIED
	// syscall_fd_resize: correct args => correct offset
	// syscall_fd_resize: shrink to zero => adjust offset correctly
	TEST_FUNC("syscall_fd_stat");
	// syscall_fd_stat: invalid buffer_length => ERROR_INVALID_ARGUMENT(2)
	// syscall_fd_stat: invalid buffer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_stat: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_stat: no FD_ACL_FLAG_STAT => ERROR_DENIED
	// syscall_fd_stat: correct args => !IS_ERROR(...) => correct data returned
	TEST_FUNC("syscall_fd_dup");
	// syscall_fd_dup: UNIMPLEMENTED
	TEST_FUNC("syscall_fd_path");
	// syscall_fd_path: invalid buffer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_path: invalid buffer_length => ERROR_NO_SPACE
	// syscall_fd_path: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_path: no FD_ACL_FLAG_STAT => ERROR_DENIED
	// syscall_fd_path: no space => ERROR_NO_SPACE
	// syscall_fd_path: correct args => !IS_ERROR(...) => correct data returned
	TEST_FUNC("syscall_fd_iter_start");
	// syscall_fd_iter_start: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_iter_start: no FD_ACL_FLAG_STAT => ERROR_DENIED
	// syscall_fd_iter_start: no read permission => ERROR_DENIED
	// syscall_fd_iter_start: correct args, no children => ERROR_NO_DATA
	// syscall_fd_iter_start: correct args, children => handle
	TEST_FUNC("syscall_fd_iter_get");
	// syscall_fd_iter_get: invalid buffer => ERROR_INVALID_ARGUMENT(1)
	// syscall_fd_iter_get: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_iter_get: no FD_ITERATOR_ACL_FLAG_ACCESS => ERROR_DENIED
	// syscall_fd_iter_get: correct args => !IS_ERROR(...) => correct return value
	TEST_FUNC("syscall_fd_iter_next");
	// syscall_fd_iter_next: invalid handle => ERROR_INVALID_HANDLE
	// syscall_fd_iter_next: no FD_ITERATOR_ACL_FLAG_ACCESS => ERROR_DENIED
	// syscall_fd_iter_next: correct args, no next child => ERROR_NO_DATA
	// syscall_fd_iter_next: correct args, next child => same handle
	TEST_FUNC("syscall_fd_iter_stop");
	// syscall_fd_iter_stop: UNIMPLEMENTE
	mmap_dealloc_region(&(THREAD_DATA->process->mmap),temp_mmap_region);
}



void test_fd(void){
	TEST_MODULE("fd");
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-cpu-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
