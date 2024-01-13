#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
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
	u8 test_buffer[TEST_ALLOC_MAX_SIZE];
	if (sys_fd_read(random_fd,test_buffer,TEST_ALLOC_MAX_SIZE,0)!=TEST_ALLOC_MAX_SIZE){
		goto _error;
	}
	for (u32 i=0;i<TEST_COUNT;i++){
		if (sys_fd_read(random_fd,&((tests+i)->size),sizeof(u64),0)!=sizeof(u64)){
			goto _error;
		}
		(tests+i)->size=((tests+i)->size%(TEST_ALLOC_MAX_SIZE-TEST_ALLOC_MIN_SIZE+1))+TEST_ALLOC_MIN_SIZE;
		(tests+i)->ptr=sys_heap_alloc(NULL,(tests+i)->size);
		sys_memory_copy(test_buffer,(tests+i)->ptr,(tests+i)->size);
	}
	for (u32 i=0;i<(TEST_COUNT>>1);i++){
		u32 j=0;
		if (sys_fd_read(random_fd,&j,sizeof(u32),0)!=sizeof(u32)){
			goto _error;
		}
		j%=TEST_COUNT;
		sys_heap_dealloc(NULL,(tests+j)->ptr);
		(tests+j)->size=0;
		(tests+j)->ptr=NULL;
		// if (sys_fd_read(random_fd,&((tests+j)->size),sizeof(u64),0)!=sizeof(u64)){
		// 	goto _error;
		// }
		// (tests+j)->size=((tests+j)->size%(TEST_ALLOC_MAX_SIZE-TEST_ALLOC_MIN_SIZE+1))+TEST_ALLOC_MIN_SIZE;
		// (tests+j)->ptr=sys_heap_alloc(NULL,(tests+j)->size);
		// sys_memory_copy(test_buffer,(tests+j)->ptr,(tests+j)->size);
	}
	for (u32 i=0;i<(TEST_COUNT>>1);i++){
		u32 j=0;
		if (sys_fd_read(random_fd,&j,sizeof(u32),0)!=sizeof(u32)){
			goto _error;
		}
		j%=TEST_COUNT;
		u64 new_size;
		if (sys_fd_read(random_fd,&new_size,sizeof(u64),0)!=sizeof(u64)){
			goto _error;
		}
		new_size=(new_size%(TEST_ALLOC_MAX_SIZE-TEST_ALLOC_MIN_SIZE+1))+TEST_ALLOC_MIN_SIZE;
		(tests+j)->ptr=sys_heap_realloc(NULL,(tests+j)->ptr,new_size);
		if (new_size>(tests+j)->size){
			sys_memory_copy(test_buffer+(tests+j)->size,(tests+j)->ptr+(tests+j)->size,new_size-(tests+j)->size);
		}
		(tests+j)->size=new_size;
	}
	for (u32 i=0;i<TEST_COUNT;i++){
		if (!(tests+i)->ptr){
			continue;
		}
		if (sys_memory_compare((tests+i)->ptr,test_buffer,(tests+i)->size)){
			sys_io_print("alloctest: failed data integrity test in block #%u\n",i);
		}
		sys_heap_dealloc(NULL,(tests+i)->ptr);
	}
	sys_fd_close(random_fd);
	u64 end_time=sys_clock_get_time_ns();
	sys_io_print("alloctest: took %lu ms\n",(end_time-start_time)/1000000);
	return 0;
_error:
	sys_fd_close(random_fd);
	sys_io_print("alloctest: unable to read from random file\n");
	return 1;
}
