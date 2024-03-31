#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



#define TEST_COUNT 256

#define TEST_ALLOC_MAX_SIZE 16384



typedef struct _ALLOCATOR_TEST_DATA{
	u64 size;
	u8* ptr;
} allocator_test_data_t;



static void _verify_integrity(const allocator_test_data_t* test_data,const u8* test_buffer){
	for (u32 i=0;i<TEST_COUNT;i++){
		if (!(test_data+i)->ptr){
			continue;
		}
		for (u64 j=0;j<(test_data+i)->size;j++){
			TEST_ASSERT((test_data+i)->ptr[j]==test_buffer[j]);
		}
	}
}



void test_amm(void){
	TEST_MODULE("amm");
	allocator_test_data_t test_data[TEST_COUNT];
	u8 test_buffer[TEST_ALLOC_MAX_SIZE];
	random_generate(test_buffer,TEST_ALLOC_MAX_SIZE);
	TEST_FUNC("amm_alloc");
	TEST_GROUP("empty chunk");
	TEST_ASSERT(!amm_alloc(0));
	TEST_GROUP("allocation");
	for (u32 i=0;i<TEST_COUNT;i++){
		random_generate(&((test_data+i)->size),sizeof(u64));
		(test_data+i)->size=(test_data+i)->size%(TEST_ALLOC_MAX_SIZE+1);
		(test_data+i)->ptr=amm_alloc((test_data+i)->size);
		TEST_ASSERT(!(test_data+i)->size||(test_data+i)->ptr);
		mem_copy((test_data+i)->ptr,test_buffer,(test_data+i)->size);
	}
	_verify_integrity(test_data,test_buffer);
	TEST_FUNC("amm_dealloc");
	TEST_GROUP("NULL pointer");
	amm_dealloc(NULL);
	_verify_integrity(test_data,test_buffer);
	TEST_GROUP("deallocation");
	for (u32 i=0;i<(TEST_COUNT>>2);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		amm_dealloc((test_data+j)->ptr);
		(test_data+j)->size=0;
		(test_data+j)->ptr=NULL;
	}
	_verify_integrity(test_data,test_buffer);
	TEST_FUNC("amm_realloc");
	TEST_GROUP("deallocation");
	for (u32 i=0;i<(TEST_COUNT>>2);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		TEST_ASSERT(!amm_realloc((test_data+j)->ptr,0));
		(test_data+j)->size=0;
		(test_data+j)->ptr=NULL;
	}
	_verify_integrity(test_data,test_buffer);
	TEST_GROUP("allocation and reallocation");
	for (u32 i=0;i<(TEST_COUNT>>1);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		u64 new_size;
		random_generate(&new_size,sizeof(u64));
		new_size=new_size%(TEST_ALLOC_MAX_SIZE+1);
		(test_data+j)->ptr=amm_realloc((test_data+j)->ptr,new_size);
		if (new_size>(test_data+j)->size){
			mem_copy((test_data+j)->ptr+(test_data+j)->size,test_buffer+(test_data+j)->size,new_size-(test_data+j)->size);
		}
		(test_data+j)->size=new_size;
	}
	_verify_integrity(test_data,test_buffer);
	for (u32 i=0;i<TEST_COUNT;i++){
		amm_dealloc((test_data+i)->ptr);
	}
}
