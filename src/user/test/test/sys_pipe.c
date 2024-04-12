#include <sys/fd/fd.h>
#include <sys/memory/memory.h>
#include <sys/pipe/pipe.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



void test_sys_pipe(void){
	TEST_MODULE("sys_pipe");
	TEST_FUNC("sys_pipe_create");
	TEST_GROUP("empty path");
	char buffer[8192]={0};
	TEST_ASSERT(sys_pipe_create(buffer)==SYS_ERROR_INVALID_ARGUMENT(0));
	TEST_GROUP("path too long");
	sys_memory_set(buffer,8192,'A');
	TEST_ASSERT(sys_pipe_create(buffer)==SYS_ERROR_INVALID_ARGUMENT(0));
	TEST_GROUP("already present");
	sys_string_copy("/dev",buffer);
	TEST_ASSERT(sys_pipe_create(buffer)==SYS_ERROR_ALREADY_PRESENT);
	TEST_GROUP("not found");
	sys_string_copy("/invalid/path/",buffer);
	TEST_ASSERT(sys_pipe_create(buffer)==SYS_ERROR_NOT_FOUND);
	// TEST_GROUP("create named");
	// sys_string_copy("/test-pipe",buffer);
	// error_t pipe_fd=sys_pipe_create(buffer);
	// TEST_ASSERT(!IS_SYS_ERROR(pipe_fd));
	// vfs_node_t* pipe=fd_get_node(pipe_fd);
	// TEST_ASSERT(pipe);
	// TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	// TEST_ASSERT(vfs_lookup(NULL,"/test-pipe",0,0,0)==pipe);
	// TEST_ASSERT(sys_fd_close(pipe_fd)==SYS_ERROR_OK);
	// vfs_node_dettach_child(pipe);
	// vfs_node_delete(pipe);
	// TEST_GROUP("create unnamed");
	// pipe_fd=sys_pipe_create(NULL);
	// TEST_ASSERT(!IS_SYS_ERROR(pipe_fd));
	// pipe=fd_get_node(pipe_fd);
	// TEST_ASSERT(pipe);
	// TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	// TEST_ASSERT(sys_fd_close(pipe_fd)==SYS_ERROR_OK);
	// vfs_node_dettach_child(pipe);
	// vfs_node_delete(pipe);
}
