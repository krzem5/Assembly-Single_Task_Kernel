#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/lib/lib.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/string/string.h>
#include <sys/system/system.h>
#include <sys/types.h>



extern void __sys_linker_dump_coverage(void) __attribute__((weak));



#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			sys_io_print("%s: assertion failed\n",#x); \
		} \
	} while (0)



void test_sys_lib(void){
	_Bool found_libld=0;
	_Bool found_libsys=0;
	_Bool found_test=0;
	for (sys_library_t library=sys_lib_get_root();library;library=sys_lib_get_next(library)){
		const char* path=sys_lib_get_path(library);
		if (!sys_string_compare(path,"/lib/ld.so")){
			TEST_ASSERT(!found_libld);
			TEST_ASSERT(sys_lib_get_image_base(library));
			found_libld=1;
		}
		else if (!sys_string_compare(path,"/lib/libsys.so")){
			TEST_ASSERT(!found_libsys);
			TEST_ASSERT(sys_lib_get_image_base(library));
			found_libsys=1;
		}
		else if (!sys_string_compare(path,"/bin/test")){
			TEST_ASSERT(!found_test);
			TEST_ASSERT(!sys_lib_get_image_base(library));
			found_test=1;
		}
		else{
			TEST_ASSERT(!"invalid library loaded");
		}
	}
	TEST_ASSERT(found_libld&&found_libsys&&found_test);
}



void main(void){
	const char*const argv[2]={
		"/bin/tree",
		"/share/test"
	};
	sys_thread_await_event(sys_process_get_termination_event(sys_process_start("/bin/tree",2,argv,NULL,0)));
	test_sys_lib();
	__sys_linker_dump_coverage();
	sys_system_shutdown(0);
}
