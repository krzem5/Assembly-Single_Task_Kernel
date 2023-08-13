#include <command.h>
#include <user/cpu.h>
#include <user/io.h>
#include <user/lock.h>
#include <user/types.h>



static lock_t _test_lock;
static volatile u8 counter;
static volatile u8 counter2;



static void core2(void* arg){
	counter++;
	lock_acquire_exclusive(&_test_lock);
	printf("core2.acq [%x]\n",_test_lock);
	printf("core2.rel\n");
	lock_release_exclusive(&_test_lock);
	counter--;
	while (counter);
	lock_acquire_shared(&_test_lock);
	printf("core2.acq++ [%x]\n",_test_lock);
	counter2++;
	while (counter2<2);
	printf("core2.acq--\n");
	lock_release_shared(&_test_lock);
}



static void core3(void* arg){
	counter++;
	lock_acquire_exclusive(&_test_lock);
	printf("core3.acq [%x]\n",_test_lock);
	printf("core3.rel\n");
	lock_release_exclusive(&_test_lock);
	counter--;
	while (counter);
	lock_acquire_shared(&_test_lock);
	printf("core3.acq++ [%x]\n",_test_lock);
	counter2++;
	while (counter2<2);
	printf("core3.acq--\n");
	lock_release_shared(&_test_lock);
}



void test_main(int argc,const char*const* argv){
	lock_init(&_test_lock);
	counter=0;
	counter2=0;
	lock_acquire_exclusive(&_test_lock);
	printf("core1.acq\n");
	cpu_core_start(2,core2,NULL);
	cpu_core_start(3,core3,NULL);
	while (counter<2);
	printf("core1.rel\n");
	lock_release_exclusive(&_test_lock);
	while (counter);
	while (counter2<2);
	lock_acquire_exclusive(&_test_lock);
	printf("core1.acq\n");
	printf("core1.rel\n");
	lock_release_exclusive(&_test_lock);
}



DECLARE_COMMAND(test,"test");
