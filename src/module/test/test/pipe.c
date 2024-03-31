#include <kernel/error/error.h>
#include <kernel/fd/fd.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/pipe/pipe.h>
#include <kernel/random/random.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kernel/vfs/vfs.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



#define TEST_BUFFER_SIZE 64



extern error_t syscall_fd_close();
extern error_t syscall_pipe_create();



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(THREAD_DATA->process->mmap,0,2*PAGE_SIZE,MMAP_REGION_FLAG_VMM_WRITE|MMAP_REGION_FLAG_VMM_USER,NULL);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	TEST_FUNC("syscall_pipe_create");
	TEST_GROUP("empty path");
	buffer[0]=0;
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_INVALID_ARGUMENT(0));
	TEST_GROUP("path too long");
	mem_fill(buffer,2*PAGE_SIZE,'A');
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_INVALID_ARGUMENT(0));
	TEST_GROUP("already present");
	str_copy("/dev",buffer,2*PAGE_SIZE);
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_ALREADY_PRESENT);
	TEST_GROUP("not found");
	str_copy("/invalid/path/",buffer,2*PAGE_SIZE);
	TEST_ASSERT(syscall_pipe_create(buffer)==ERROR_NOT_FOUND);
	TEST_GROUP("create named");
	str_copy("/test-pipe",buffer,2*PAGE_SIZE);
	error_t pipe_fd=syscall_pipe_create(buffer);
	TEST_ASSERT(!IS_ERROR(pipe_fd));
	vfs_node_t* pipe=fd_get_node(pipe_fd);
	TEST_ASSERT(pipe);
	TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	TEST_ASSERT(vfs_lookup(NULL,"/test-pipe",0,0,0)==pipe);
	TEST_ASSERT(syscall_fd_close(pipe_fd)==ERROR_OK);
	vfs_node_dettach_external_child(pipe);
	vfs_node_delete(pipe);
	TEST_GROUP("create unnamed");
	pipe_fd=syscall_pipe_create(NULL);
	TEST_ASSERT(!IS_ERROR(pipe_fd));
	pipe=fd_get_node(pipe_fd);
	TEST_ASSERT(pipe);
	TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	TEST_ASSERT(syscall_fd_close(pipe_fd)==ERROR_OK);
	vfs_node_dettach_external_child(pipe);
	vfs_node_delete(pipe);
	mmap_dealloc_region(THREAD_DATA->process->mmap,temp_mmap_region);
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
	vfs_node_dettach_external_child(pipe);
	vfs_node_delete(pipe);
	TEST_GROUP("create unnamed");
	pipe=pipe_create(NULL,NULL);
	TEST_ASSERT(pipe);
	TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	TEST_FUNC("_pipe_read");
	TEST_GROUP("empty buffer");
	TEST_ASSERT(!vfs_node_read(pipe,0,NULL,0,0));
	TEST_GROUP("empty nonblocking read");
	char buffer[TEST_BUFFER_SIZE];
	TEST_ASSERT(!vfs_node_read(pipe,0,buffer,TEST_BUFFER_SIZE,VFS_NODE_FLAG_NONBLOCKING));
	TEST_GROUP("blocking read");
	random_generate(buffer,TEST_BUFFER_SIZE);
	TEST_ASSERT(vfs_node_write(pipe,0,buffer,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	char buffer2[TEST_BUFFER_SIZE];
	mem_fill(buffer2,TEST_BUFFER_SIZE,0);
	TEST_ASSERT(vfs_node_read(pipe,0,buffer2,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
		TEST_ASSERT(buffer2[i]==buffer[i]);
	}
	TEST_GROUP("peek");
	random_generate(buffer,TEST_BUFFER_SIZE);
	TEST_ASSERT(vfs_node_write(pipe,0,buffer,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	mem_fill(buffer2,TEST_BUFFER_SIZE,0);
	TEST_ASSERT(vfs_node_read(pipe,0,buffer2,TEST_BUFFER_SIZE,VFS_NODE_FLAG_PIPE_PEEK)==TEST_BUFFER_SIZE);
	for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
		TEST_ASSERT(buffer2[i]==buffer[i]);
	}
	mem_fill(buffer2,TEST_BUFFER_SIZE,0);
	TEST_ASSERT(vfs_node_read(pipe,0,buffer2,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
		TEST_ASSERT(buffer2[i]==buffer[i]);
	}
	TEST_FUNC("_pipe_write");
	TEST_GROUP("empty buffer");
	TEST_ASSERT(!vfs_node_write(pipe,0,NULL,0,0));
	TEST_GROUP("blocking write");
	for (u32 i=0;i<PIPE_BUFFER_SIZE/TEST_BUFFER_SIZE;i++){
		TEST_ASSERT(vfs_node_write(pipe,0,buffer,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	}
	TEST_ASSERT(vfs_node_write(pipe,0,buffer,PIPE_BUFFER_SIZE%TEST_BUFFER_SIZE,0)==(PIPE_BUFFER_SIZE%TEST_BUFFER_SIZE));
	TEST_GROUP("full nonblocking write");
	TEST_ASSERT(!vfs_node_write(pipe,0,buffer,PIPE_BUFFER_SIZE,VFS_NODE_FLAG_NONBLOCKING));
	vfs_node_dettach_external_child(pipe);
	vfs_node_delete(pipe);
	process_t* test_process=process_create("test-process","test-process",0x1000,0x3000);
	handle_acquire(&(test_process->handle));
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-pipe-thread",_thread,0));
	event_await(test_process->event,0);
	handle_release(&(test_process->handle));
}
