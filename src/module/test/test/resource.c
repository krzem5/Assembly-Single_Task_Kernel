#include <kernel/log/log.h>
#include <kernel/random/random.h>
#include <kernel/resource/resource.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



#define TEST_COUNT 256

#define TEST_MIN 1
#define TEST_MAX (TEST_MIN+TEST_COUNT-1)



typedef struct _ALLOCATOR_TEST_DATA{
	resource_t resource;
} allocator_test_data_t;



static void _verify_integrity(resource_manager_t* resource_manager,const allocator_test_data_t* test_data){
	for (u32 i=0;i<TEST_COUNT;i++){
		if ((test_data+i)->resource){
			TEST_ASSERT(resource_is_used(resource_manager,(test_data+i)->resource));
		}
	}
}



void test_resource(void){
	TEST_MODULE("resource");
	TEST_FUNC("resource_manager_create");
	TEST_GROUP("empty range");
	TEST_ASSERT(!resource_manager_create(5,5));
	TEST_GROUP("correct args");
	resource_manager_t* resource_manager=resource_manager_create(TEST_MIN,TEST_MAX);
	TEST_ASSERT(resource_manager);
	TEST_ASSERT(resource_manager->max==TEST_MAX);
	resource_manager_delete(resource_manager);
	TEST_FUNC("resource_manager_delete");
	TEST_GROUP("correct args");
	resource_manager=resource_manager_create(TEST_MIN,TEST_MAX);
	TEST_ASSERT(resource_manager);
	TEST_ASSERT(resource_alloc(resource_manager));
	resource_manager_delete(resource_manager);
	TEST_FUNC("resource_alloc");
	resource_manager=resource_manager_create(TEST_MIN,TEST_MAX);
	TEST_ASSERT(resource_manager);
	allocator_test_data_t test_data[TEST_COUNT];
	TEST_GROUP("allocation");
	for (u32 i=0;i<TEST_COUNT;i++){
		(test_data+i)->resource=resource_alloc(resource_manager);
		TEST_ASSERT((test_data+i)->resource);
	}
	_verify_integrity(resource_manager,test_data);
	TEST_GROUP("no space");
	TEST_ASSERT(!resource_alloc(resource_manager));
	TEST_FUNC("resource_dealloc");
	TEST_GROUP("invalid resource");
	TEST_ASSERT(!resource_dealloc(resource_manager,0));
	TEST_ASSERT(!resource_dealloc(resource_manager,TEST_MAX+1));
	_verify_integrity(resource_manager,test_data);
	TEST_GROUP("deallocation");
	for (u32 i=0;i<(TEST_COUNT>>1);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		if (!(test_data+j)->resource){
			continue;
		}
		TEST_ASSERT(resource_dealloc(resource_manager,(test_data+j)->resource));
		(test_data+j)->resource=0;
	}
	_verify_integrity(resource_manager,test_data);
	TEST_GROUP("allocation and deallocation");
	for (u32 i=0;i<(TEST_COUNT>>2);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		u8 do_alloc;
		random_generate(&do_alloc,sizeof(u8));
		do_alloc&=1;
		if (do_alloc==(!!(test_data+j)->resource)){
			continue;
		}
		if (do_alloc){
			(test_data+j)->resource=resource_alloc(resource_manager);
			TEST_ASSERT((test_data+j)->resource);
		}
		else{
			TEST_ASSERT(resource_dealloc(resource_manager,(test_data+j)->resource));
			(test_data+j)->resource=0;
		}
	}
	_verify_integrity(resource_manager,test_data);
	resource_manager_delete(resource_manager);
}
