#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/ring/ring.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



#define TEST_RING_SIZE 64
#define TEST_BUFFER_SIZE 8



void test_ring(void){
	TEST_MODULE("ring");
	TEST_FUNC("ring_create");
	TEST_GROUP("empty ring");
	TEST_ASSERT(!ring_init(0));
	TEST_GROUP("capacity not a power of 2");
	TEST_ASSERT(!ring_init(3));
	TEST_GROUP("correct args");
	ring_t* ring=ring_init(TEST_RING_SIZE);
	TEST_ASSERT(ring);
	ring_deinit(ring);
	TEST_FUNC("ring_push");
	ring=ring_init(TEST_RING_SIZE);
	void* test_buffer[TEST_BUFFER_SIZE];
	random_generate(test_buffer,TEST_BUFFER_SIZE*sizeof(void*));
	TEST_GROUP("correct args");
	for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
		TEST_ASSERT(ring_push(ring,test_buffer[i],0));
	}
	for (u32 i=0;i<TEST_BUFFER_SIZE;i++){
		TEST_ASSERT(ring_pop(ring,0)==test_buffer[i]);
	}
	TEST_GROUP("full ring");
	for (u32 i=0;i<TEST_RING_SIZE;i++){
		TEST_ASSERT(ring_push(ring,(void*)12345,0));
	}
	TEST_ASSERT(!ring_push(ring,(void*)12345,0));
	TEST_FUNC("ring_pop");
	TEST_GROUP("empty ring");
	// ring_pop: empty ring
	TEST_GROUP("correct args");
	// ring_pop: correct args
	TEST_FUNC("ring_peek");
	TEST_GROUP("empty ring");
	// ring_peek: empty ring
	TEST_GROUP("correct args");
	// ring_peek: correct args
	ring_deinit(ring);
}
