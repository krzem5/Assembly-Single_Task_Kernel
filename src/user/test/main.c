#include <sys/elf/elf.h>
#include <sys/io/io.h>
#include <sys/lib/lib.h>
#include <sys/mp/process.h>
#include <sys/mp/thread.h>
#include <sys/string/string.h>
#include <sys/system/system.h>
#include <sys/types.h>
#include <test/test.h>



extern void __sys_linker_dump_coverage(void) __attribute__((weak));



void test_sys_lib(void){
	TEST_MODULE("sys_lib");
	TEST_FUNC("sys_lib_get_root");
	TEST_GROUP("correct args");
	TEST_ASSERT(sys_lib_get_root());
	TEST_FUNC("sys_lib_get_next");
	TEST_GROUP("correct args");
	TEST_ASSERT(sys_lib_get_next(sys_lib_get_root())!=sys_lib_get_root());
	TEST_GROUP("end of chain");
	TEST_ASSERT(!sys_lib_get_next(sys_lib_get_next(sys_lib_get_next(sys_lib_get_root()))));
	TEST_FUNC("sys_lib_get_path");
	TEST_GROUP("correct args");
	TEST_ASSERT(!sys_string_compare(sys_lib_get_path(sys_lib_get_root()),"/lib/ld.so"));
	TEST_FUNC("sys_lib_get_image_base");
	TEST_GROUP("correct args");
	TEST_ASSERT(sys_lib_get_image_base(sys_lib_get_root()));
	TEST_FUNC("sys_lib_load");
	// sys_lib_load
	TEST_FUNC("sys_lib_lookup_symbol");
	// sys_lib_lookup_symbol
	TEST_FUNC("sys_lib_get_search_path");
	TEST_GROUP("correct args");
	TEST_ASSERT(!sys_string_compare(sys_lib_get_search_path(),"/lib:/:."));
	TEST_FUNC("sys_lib_set_search_path");
	TEST_GROUP("correct args");
	sys_lib_set_search_path("/lib:/:.:/new/lib/path");
	TEST_ASSERT(!sys_string_compare(sys_lib_get_search_path(),"/lib:/:.:/new/lib/path"));
	sys_lib_set_search_path("/lib:/:.");
	TEST_ASSERT(!sys_string_compare(sys_lib_get_search_path(),"/lib:/:."));
	// _Bool found_libld=0;
	// _Bool found_libsys=0;
	// _Bool found_test=0;
	// for (sys_library_t library=sys_lib_get_root();library;library=sys_lib_get_next(library)){
	// 	const char* path=sys_lib_get_path(library);
	// 	if (!sys_string_compare(path,"/lib/ld.so")){
	// 		TEST_ASSERT(!found_libld);
	// 		TEST_ASSERT(sys_lib_get_image_base(library));
	// 		found_libld=1;
	// 	}
	// 	else if (!sys_string_compare(path,"/lib/libsys.so")){
	// 		TEST_ASSERT(!found_libsys);
	// 		TEST_ASSERT(sys_lib_get_image_base(library));
	// 		found_libsys=1;
	// 	}
	// 	else if (!sys_string_compare(path,"/bin/test")){
	// 		TEST_ASSERT(!found_test);
	// 		TEST_ASSERT(!sys_lib_get_image_base(library));
	// 		found_test=1;
	// 	}
	// 	else{
	// 		TEST_ASSERT(!"invalid library loaded");
	// 	}
	// }
	// TEST_ASSERT(found_libld&&found_libsys&&found_test);
}



void main(void){
	const char*const argv[2]={
		"/bin/tree",
		"/share/test"
	};
	sys_thread_await_event(sys_process_get_termination_event(sys_process_start("/bin/tree",2,argv,NULL,0)));
	test_sys_lib();
	// u64 test_pass_count=0;
	// u64 test_fail_count=0;
	__sys_linker_dump_coverage();
	sys_system_shutdown(0);
}
