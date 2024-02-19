#ifndef _TEST_TEST_H_
#define _TEST_TEST_H_ 1
#include <sys/io/io.h>
#include <sys/types.h>



#define TEST_MODULE(name) sys_io_print("Module '%s'...\n",name);
#define TEST_FUNC(name) sys_io_print("Function '%s'...\n",name);
#define TEST_GROUP(name) sys_io_print("Group '%s'...\n",name);
#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			sys_io_print("%u: %s: Test failed\n",__LINE__,#x); \
			test_fail_count++; \
		} \
		else{ \
			test_pass_count++; \
		} \
	} while (0)



extern u64 test_pass_count;
extern u64 test_fail_count;



#endif
