#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



#define TEST_COUNT 1024

#define TEST_ALLOC_MIN_SIZE 1
#define TEST_ALLOC_MAX_SIZE 16384



typedef struct _SINGLE_TEST_DATA{
	u64 size;
	void* ptr;
} single_test_data_t;



int main(int argc,const char** argv){
	if (!sys_options_parse(argc,argv,NULL)){
		return 1;
	}
	sys_fd_t random_fd=sys_fd_open(0,"/dev/random",SYS_FD_FLAG_READ);
	if (SYS_IS_ERROR(random_fd)){
		sys_io_print("alloctest: unable to open random file: error %ld\n",random_fd);
		return 1;
	}
	u64 start_time=sys_clock_get_time_ns();
	single_test_data_t tests[TEST_COUNT];
	for (u32 i=0;i<TEST_COUNT;i++){
		if (sys_fd_read(random_fd,&((tests+i)->size),sizeof(u64),0)!=sizeof(u64)){
			goto _error;
		}
		(tests+i)->size=((tests+i)->size%(TEST_ALLOC_MAX_SIZE-TEST_ALLOC_MIN_SIZE+1))+TEST_ALLOC_MIN_SIZE;
		sys_io_print("[%u]: %u\n",i,(tests+i)->size);
		(tests+i)->ptr=sys_heap_alloc(NULL,(tests+i)->size);
	}
	// /* malloc some randomly sized blocks */
	// for (int i = 0; i < ITERATION_COUNT; ++i) {
	// 	size_t sz = RAND_SIZE();
	// 	sizes[i] = 0;
	// 	ptrs[i] = malloc(sz);
	// 	if (ptrs[i] != NULL) {
	// 		sizes[i] = sz;
	// 		chrs[i] = (char)RAND_RANGE(0, 256);
	// 		for (size_t j = 0; j < sz; ++j) {
	// 			ptrs[i][j] = chrs[i];
	// 		}
	// 	}
	// }

	// /* free some of the pointers */
	// for (int i = 0; i < ITERATION_COUNT / 2; ++i) {
	// 	int index = rand() % ITERATION_COUNT;
	// 	free(ptrs[index]);
	// 	ptrs[index] = NULL;
	// 	sizes[index] = 0;
	// 	chrs[index] = '\0';
	// }

	// /* realloc some of the pointers */
	// for (int i = 0; i < ITERATION_COUNT / 2; ++i) {
	// 	int index = rand() % ITERATION_COUNT;
	// 	size_t sz = RAND_SIZE();
	// 	void *x = realloc(ptrs[index], sz);

	// 	if (sz == 0 || x != NULL) {
	// 		ptrs[index] = x;
	// 		sizes[index] = sz;
	// 		chrs[index] = (char)RAND_RANGE(0, 256);
	// 		for (size_t j = 0; j < sz; ++j) {
	// 			ptrs[index][j] = chrs[index];
	// 		}
	// 	}
	// }

	// /* Make sure our data is still intact */
	// for (int i = 0; i < ITERATION_COUNT; ++i) {
	// 	for (int j = 0; j < sizes[i]; ++j) {
	// 		assert(ptrs[i][j] == chrs[i]);
	// 	}
	// }

	// /* Clean up our mess */
	// for (int i = 0; i < ITERATION_COUNT; ++i) {
	// 	free(ptrs[i]);
	// }
	sys_fd_close(random_fd);
	u64 end_time=sys_clock_get_time_ns();
	sys_io_print("alloctest: took %lu ms\n",(end_time-start_time)/1000000);
	return 0;
_error:
	sys_fd_close(random_fd);
	sys_io_print("alloctest: unable to read from random file\n");
	return 1;
}
