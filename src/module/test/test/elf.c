#include <kernel/elf/elf.h>
#include <kernel/elf/structures.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <test/elf.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static u32 _test_elf_expected_argc=0;
static const char*const* _test_elf_expected_argv=NULL;
static u32 _test_elf_expected_environ_length=0;
static const char*const* _test_elf_expected_environ=NULL;



static void _syscall_verify_elf_data(u32 argc,const char*const* argv,const char*const* environ,const u64* auxv){
	TEST_ASSERT(argc==_test_elf_expected_argc);
	if (argc!=_test_elf_expected_argc){
		return;
	}
	for (u32 i=0;i<argc;i++){
		TEST_ASSERT(str_equal(argv[i],_test_elf_expected_argv[i]));
	}
	u32 environ_length=0;
	for (;environ[environ_length];environ_length++);
	TEST_ASSERT(environ_length==_test_elf_expected_environ_length);
	if (environ_length!=_test_elf_expected_environ_length){
		return;
	}
	for (u32 i=0;i<environ_length;i++){
		TEST_ASSERT(str_equal(environ[i],_test_elf_expected_environ[i]));
	}
	while (auxv[0]!=AT_NULL){
		if (auxv[0]==AT_PAGESZ){
			TEST_ASSERT(auxv[1]==PAGE_SIZE);
		}
		else if (auxv[0]==AT_FLAGS){
			TEST_ASSERT(!auxv[1]);
		}
		else if (auxv[0]==AT_PLATFORM){
			TEST_ASSERT(str_equal((const char*)auxv[1],ELF_AUXV_PLATFORM));
		}
		else if (auxv[0]==AT_HWCAP){
			TEST_ASSERT(auxv[1]==test_elf_get_correct_hwcap());
		}
		else if (auxv[0]==AT_HWCAP2){
			TEST_ASSERT(!auxv[1]);
		}
		else if (auxv[0]==AT_EXECFN){
			TEST_ASSERT(str_equal((const char*)auxv[1],"/bin/test_elf_send_results"));
		}
		auxv+=2;
	}
}



static syscall_callback_t const _test_elf_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_verify_elf_data
};



static void _test_arguments(u32 argc,const char*const* argv,u32 environ_length,const char*const* environ){
	const char* path="/bin/test_elf_send_results";
	error_t ret=elf_load(path,argc,argv,environ_length,environ,0);
	if (!argc){
		argc=1;
		argv=&path;
	}
	_test_elf_expected_argc=argc;
	_test_elf_expected_argv=argv;
	_test_elf_expected_environ_length=environ_length;
	_test_elf_expected_environ=environ;
	TEST_ASSERT(!IS_ERROR(ret));
	if (!IS_ERROR(ret)){
		handle_t* handle=handle_lookup_and_acquire(ret,process_handle_type);
		process_t* process=handle->object;
		event_await(process->event,0);
		handle_release(handle);
	}
}



void test_elf(void){
	TEST_MODULE("elf");
	TEST_FUNC("elf_load");
	TEST_GROUP("invalid path");
	TEST_ASSERT(elf_load("/invalid/path",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("no read and execute permissions");
	TEST_ASSERT(elf_load("/share/test/elf/no_read_and_execute_access_file",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("no execute permissions");
	TEST_ASSERT(elf_load("/share/test/elf/no_execute_access_file",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("no read permissions");
	TEST_ASSERT(elf_load("/share/test/elf/no_read_access_file",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("invalid header");
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_signature",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_word_size",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_endianess",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_abi",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_type",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_machine",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/invalid_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_GROUP("multiple interpreters");
	TEST_ASSERT(elf_load("/share/test/elf/multiple_interpreters",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_GROUP("unterminated interpreter");
	TEST_ASSERT(elf_load("/share/test/elf/unterminated_interpreter",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_GROUP("invalid interpreter path");
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_path",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("no read and execute interpreter permissions");
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_no_read_and_execute_permissions",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("no execute interpreter permissions");
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_no_execute_permissions",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("no read interpreter permissions");
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_no_read_permissions",0,NULL,0,NULL,0)==ERROR_NOT_FOUND);
	TEST_GROUP("invalid interpreter header");
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_signature",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_word_size",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_endianess",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_abi",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_type",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_machine",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_ASSERT(elf_load("/share/test/elf/interpreter_invalid_header_version",0,NULL,0,NULL,0)==ERROR_INVALID_FORMAT);
	TEST_GROUP("no args");
	syscall_create_table("test_elf",_test_elf_syscall_functions,sizeof(_test_elf_syscall_functions)/sizeof(syscall_callback_t));
	_test_arguments(0,NULL,0,NULL);
	TEST_GROUP("correct args");
	const char*const argv[]={
		"/bin/other_executable",
		"arg0",
		"=\"",
		"quoted-string",
		"\\\\\\",
		"333"
	};
	const char*const environ[]={
		"PATH=/bin:/",
		"PWD=/",
		"HOME=/",
		"USER=root"
	};
	_test_arguments(sizeof(argv)/sizeof(const char*),argv,sizeof(environ)/sizeof(const char*),environ);
}
