#include <kernel/log/log.h>
#include <kernel/format/format.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test_format"



#define TEST_FORMAT_BUFFER_SIZE 128



static void _test_format_empty(const char* marker,...){
	__builtin_va_list va;
	__builtin_va_start(va,marker);
	TEST_ASSERT(!format_string_va(NULL,0,"",&va));
	__builtin_va_end(va);
}



static void _test_format(const char* expected,const char* template,...){
	char buffer[TEST_FORMAT_BUFFER_SIZE];
	__builtin_va_list va;
	__builtin_va_start(va,template);
	format_string_va(buffer,TEST_FORMAT_BUFFER_SIZE,template,&va);
	__builtin_va_end(va);
	TEST_ASSERT(streq(buffer,expected));
}



void test_format(void){
	LOG("Executing format tests...");
	_test_format_empty("");
	_test_format("","");
	_test_format("The quick brown fox jumps over the lazy dog","The quick brown fox jumps over the lazy dog");
	_test_format("N","%c",'N');
	_test_format("%","%c",'%');
	_test_format("(null) | The quick brown fox...","%s | %s",NULL,"The quick brown fox...");
	// d,u,x
	_test_format("00,4b","%X,%X",0,0x4b);
	_test_format("0 B,1 B,2 KB,3 MB,4 GB,5 TB,6 PB,7 EB","%v,%v,%v,%v,%v,%v,%v,%v",0,1,0x800,0x300000,0x100000000,0x50000000000,0x18000000000000,0x7000000000000000);
	_test_format("00000000_00000000,11223344_aabbccdd","%p,%p",NULL,(void*)0x11223344aabbccdd);
	u8 null_guid[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 test_guid[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
	_test_format("00000000-0000-0000-0000-000000000000,00112233-4455-6677-8899-aabbccddeeff","%g,%g",null_guid,test_guid);
}
