#ifndef _COVERAGE_TEST_TEST_H_
#define _COVERAGE_TEST_TEST_H_ 1
#include <kernel/types.h>



#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			ERROR("%u: %s: Test failed",__LINE__,#x); \
			coverage_test_fail_count++; \
		} \
		else{ \
			coverage_test_pass_count++; \
		} \
	} while (0)



extern u64 coverage_test_pass_count;
extern u64 coverage_test_fail_count;



#endif
