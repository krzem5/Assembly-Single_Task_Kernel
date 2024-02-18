#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/random/random.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



#define TEST_COUNT 256

#define TEST_ALLOC_MIN_SIZE 1
// #define TEST_ALLOC_MAX_SIZE 16384
#define TEST_ALLOC_MAX_SIZE 4095



typedef struct _SINGLE_TEST_DATA{
	u64 size;
	u8* ptr;
} single_test_data_t;



static void _verify_integrity(const single_test_data_t* tests,const u8* test_buffer){
	for (u32 i=0;i<TEST_COUNT;i++){
		if (!(tests+i)->ptr){
			continue;
		}
		for (u64 i=0;i<(tests+i)->size;i++){
			TEST_ASSERT((tests+i)->ptr[i]==test_buffer[i]);
		}
	}
}



void test_amm(void){
	TEST_MODULE("amm");
	single_test_data_t tests[TEST_COUNT];
	u8 test_buffer[TEST_ALLOC_MAX_SIZE];
	random_generate(test_buffer,TEST_ALLOC_MAX_SIZE);
	TEST_FUNC("amm_alloc");
	TEST_GROUP("empty chunk");
	TEST_ASSERT(!amm_alloc(0));
	TEST_GROUP("allocation");
	for (u32 i=0;i<TEST_COUNT;i++){
		random_generate(&((tests+i)->size),sizeof(u64));
		(tests+i)->size=((tests+i)->size%(TEST_ALLOC_MAX_SIZE-TEST_ALLOC_MIN_SIZE+1))+TEST_ALLOC_MIN_SIZE;
		(tests+i)->ptr=amm_alloc((tests+i)->size);
		memcpy((tests+i)->ptr,test_buffer,(tests+i)->size);
		TEST_ASSERT((tests+i)->ptr);
	}
	_verify_integrity(tests,test_buffer);
	TEST_FUNC("amm_dealloc");
	TEST_GROUP("NULL pointer");
	amm_dealloc(NULL);
	_verify_integrity(tests,test_buffer);
	TEST_GROUP("deallocation");
	for (u32 i=0;i<(TEST_COUNT>>1);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		amm_dealloc((tests+j)->ptr);
		(tests+j)->size=0;
		(tests+j)->ptr=NULL;
	}
	_verify_integrity(tests,test_buffer);
	TEST_FUNC("amm_realloc");
	TEST_GROUP("allocation");
	_verify_integrity(tests,test_buffer);
	TEST_GROUP("deallocation");
	_verify_integrity(tests,test_buffer);
	TEST_GROUP("reallocation");
	for (u32 i=0;i<(TEST_COUNT>>1);i++){
		u32 j=0;
		random_generate(&j,sizeof(u32));
		j%=TEST_COUNT;
		u64 new_size;
		random_generate(&new_size,sizeof(u64));
		new_size=(new_size%(TEST_ALLOC_MAX_SIZE-TEST_ALLOC_MIN_SIZE+1))+TEST_ALLOC_MIN_SIZE;
		(tests+j)->ptr=amm_realloc((tests+j)->ptr,new_size);
		if (new_size>(tests+j)->size){
			memcpy((tests+j)->ptr+(tests+j)->size,test_buffer+(tests+j)->size,new_size-(tests+j)->size);
		}
		(tests+j)->size=new_size;
	}
	_verify_integrity(tests,test_buffer);
	for (u32 i=0;i<TEST_COUNT;i++){
		amm_dealloc((tests+i)->ptr);
	}
}
