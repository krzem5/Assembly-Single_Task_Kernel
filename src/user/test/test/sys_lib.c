#include <sys/lib/lib.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



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
	TEST_GROUP("not found");
	TEST_ASSERT(!sys_lib_load("/invalid/path",0));
	TEST_GROUP("invalid header");
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_signature",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_word_size",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_endianess",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_header_version",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_abi",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_type",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_machine",0));
	TEST_ASSERT(!sys_lib_load("/share/test/sys_lib/invalid_header_version",0));
	// sys_lib_load
	TEST_FUNC("sys_lib_lookup_symbol");
	TEST_GROUP("symbol not found");
	TEST_ASSERT(!sys_lib_lookup_symbol(0,"symbol_not_found"));
	// TEST_GROUP("correct args");
	// sys_io_print("%p %p\n",sys_lib_lookup_symbol(0,"test_sys_lib"),test_sys_lib);
	// TEST_ASSERT(sys_lib_lookup_symbol(0,"test_sys_lib")==test_sys_lib);
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
}
