#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_fd_close();
extern error_t syscall_pipe_create();



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(&(THREAD_DATA->process->mmap),0,2*PAGE_SIZE,NULL,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL,0);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	TEST_FUNC("syscall_pipe_create");
	TEST_GROUP("empty path");
	buffer[0]=0;
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_INVALID_ARGUMENT(0));
	TEST_GROUP("path too long");
	memset(buffer,'A',2*PAGE_SIZE);
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_INVALID_ARGUMENT(0));
	TEST_GROUP("already present");
	strcpy(buffer,"/dev",2*PAGE_SIZE);
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_ALREADY_PRESENT);
	TEST_GROUP("not found");
	strcpy(buffer,"/invalid/path/",2*PAGE_SIZE);
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_NOT_FOUND);
	TEST_GROUP("create named");
	// strcpy(buffer,"/test-pipe",2*PAGE_SIZE);
	// error_t pipe_fd=syscall_pipe_create(buffer);
	// TEST_ASSERT(!IS_ERROR(pipe_fd));
	// vfs_node_t* pipe=fd_get_node(pipe_fd);
	// TEST_ASSERT(pipe);
	// TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	// TEST_ASSERT(vfs_lookup(NULL,"/test-pipe",0,0,0)==pipe);
	// TEST_ASSERT(syscall_fd_close(pipe_fd)==ERROR_OK);
	// vfs_node_dettach_child(pipe);
	// vfs_node_delete(pipe);
	TEST_GROUP("create unnamed");
	// error_t pipe_fd=syscall_pipe_create(NULL);
	// TEST_ASSERT(!IS_ERROR(pipe_fd));
	// vfs_node_t* pipe=fd_get_node(pipe_fd);
	// TEST_ASSERT(pipe);
	// TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	// vfs_node_dettach_child(pipe);
	// vfs_node_delete(pipe);
	mmap_dealloc_region(&(THREAD_DATA->process->mmap),temp_mmap_region);
}



void test_pipe(void){
	TEST_MODULE("pipe");
	TEST_FUNC("pipe_create");
	TEST_GROUP("create named");
	SMM_TEMPORARY_STRING name=smm_alloc("test-pipe",0);
	vfs_node_t* pipe=pipe_create(vfs_lookup(NULL,"/",0,0,0),name);
	TEST_ASSERT(pipe);
	TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	TEST_ASSERT(vfs_lookup(NULL,"/test-pipe",0,0,0)==pipe);
	vfs_node_dettach_child(pipe);
	vfs_node_delete(pipe);
	TEST_GROUP("create unnamed");
	pipe=pipe_create(NULL,NULL);
	TEST_ASSERT(pipe);
	TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	vfs_node_dettach_child(pipe);
	vfs_node_delete(pipe);
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
