#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/ring/ring.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



#define TEST_BUFFER_SIZE 64



void test_ring(void){
	TEST_MODULE("ring");
	TEST_FUNC("ring_create");
	TEST_GROUP("empty ring");
	TEST_ASSERT(!ring_init(0));
	TEST_GROUP("capacity not a power of 2");
	TEST_ASSERT(!ring_init(3));
	// TEST_GROUP("create named");
	// SMM_TEMPORARY_STRING name=smm_alloc("test-pipe",0);
	// vfs_node_t* pipe=pipe_create(vfs_lookup(NULL,"/",0,0,0),name);
	// TEST_ASSERT(pipe);
	// TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	// TEST_ASSERT(vfs_lookup(NULL,"/test-pipe",0,0,0)==pipe);
	// vfs_node_dettach_external_child(pipe);
	// vfs_node_delete(pipe);
	// TEST_GROUP("create unnamed");
	// pipe=pipe_create(NULL,NULL);
	// TEST_ASSERT(pipe);
	// TEST_ASSERT((pipe->flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_PIPE);
	// TEST_FUNC("_pipe_read");
	// TEST_GROUP("empty buffer");
	// TEST_ASSERT(!vfs_node_read(pipe,0,NULL,0,0));
	// TEST_GROUP("empty nonblocking read");
	// char buffer[TEST_BUFFER_SIZE];
	// TEST_ASSERT(!vfs_node_read(pipe,0,buffer,TEST_BUFFER_SIZE,VFS_NODE_FLAG_NONBLOCKING));
	// TEST_GROUP("blocking read");
	// random_generate(buffer,TEST_BUFFER_SIZE);
	// TEST_ASSERT(vfs_node_write(pipe,0,buffer,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	// char buffer2[TEST_BUFFER_SIZE];
	// memset(buffer2,0,TEST_BUFFER_SIZE);
	// TEST_ASSERT(vfs_node_read(pipe,0,buffer2,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	// for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
	// 	TEST_ASSERT(buffer2[i]==buffer[i]);
	// }
	// TEST_GROUP("peek");
	// random_generate(buffer,TEST_BUFFER_SIZE);
	// TEST_ASSERT(vfs_node_write(pipe,0,buffer,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	// memset(buffer2,0,TEST_BUFFER_SIZE);
	// TEST_ASSERT(vfs_node_read(pipe,0,buffer2,TEST_BUFFER_SIZE,VFS_NODE_FLAG_PIPE_PEEK)==TEST_BUFFER_SIZE);
	// for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
	// 	TEST_ASSERT(buffer2[i]==buffer[i]);
	// }
	// memset(buffer2,0,TEST_BUFFER_SIZE);
	// TEST_ASSERT(vfs_node_read(pipe,0,buffer2,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	// for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
	// 	TEST_ASSERT(buffer2[i]==buffer[i]);
	// }
	// TEST_FUNC("_pipe_write");
	// TEST_GROUP("empty buffer");
	// TEST_ASSERT(!vfs_node_write(pipe,0,NULL,0,0));
	// TEST_GROUP("blocking write");
	// for (u32 i=0;i<PIPE_BUFFER_SIZE/TEST_BUFFER_SIZE;i++){
	// 	TEST_ASSERT(vfs_node_write(pipe,0,buffer,TEST_BUFFER_SIZE,0)==TEST_BUFFER_SIZE);
	// }
	// TEST_ASSERT(vfs_node_write(pipe,0,buffer,PIPE_BUFFER_SIZE%TEST_BUFFER_SIZE,0)==(PIPE_BUFFER_SIZE%TEST_BUFFER_SIZE));
	// TEST_GROUP("full nonblocking write");
	// TEST_ASSERT(!vfs_node_write(pipe,0,buffer,PIPE_BUFFER_SIZE,VFS_NODE_FLAG_NONBLOCKING));
	// vfs_node_dettach_external_child(pipe);
	// vfs_node_delete(pipe);
}
