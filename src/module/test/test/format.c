#include <kernel/format/format.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



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
	TEST_ASSERT(str_equal(buffer,expected));
}



void test_format(void){
	TEST_MODULE("format");
	TEST_FUNC("format_string_va");
	TEST_GROUP("empty buffer");
	_test_format_empty("");
	TEST_GROUP("empty template");
	_test_format("","");
	TEST_GROUP("unterminated escape sequence");
	_test_format("","%");
	TEST_GROUP("only text");
	_test_format("The quick brown fox jumps over the lazy dog","The quick brown fox jumps over the lazy dog");
	TEST_GROUP("character (%c)");
	_test_format("N","%c",'N');
	_test_format("%","%c",'%');
	TEST_GROUP("string (%s)");
	_test_format("(null) | The quick brown fox...","%s | %s",NULL,"The quick brown fox...");
	TEST_GROUP("signed number (%d)");
	_test_format("0,-1","%d,%d",0,-1);
	_test_format("-22,-22,-22","%hhd,%hd,%ld",0x88ea,0x8888ffea,0xffffffffffffffea);
	TEST_GROUP("unsigned number (%u)");
	_test_format("0,1","%u,%u",0,1);
	_test_format("22,22,22","%hhu,%hu,%lu",0x8816,0x88880016,0x16);
	TEST_GROUP("unsigned hexadecimal number (%x)");
	_test_format("0,1","%x,%x",0,1);
	_test_format("22,22,22","%hhx,%hx,%lx",0x8822,0x88880022,0x22);
	TEST_GROUP("8-bit hexadecimal number (%X)");
	_test_format("00,4b","%X,%X",0,0x4b);
	TEST_GROUP("byte size (%v)");
	_test_format("0 B,1 B,2 KB,3 MB,4 GB,5 TB,6 PB,7 EB","%v,%v,%v,%v,%v,%v,%v,%v",0,1,0x800,0x300000,0x100000000,0x50000000000,0x18000000000000,0x7000000000000000);
	TEST_GROUP("pointer (%p)");
	_test_format("00000000_00000000,11223344_aabbccdd","%p,%p",NULL,(void*)0x11223344aabbccdd);
	TEST_GROUP("uuid (%g)");
	u8 null_uuid[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 test_uuid[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
	_test_format("00000000-0000-0000-0000-000000000000,00112233-4455-6677-8899-aabbccddeeff","%g,%g",null_uuid,test_uuid);
	TEST_GROUP("MAC address (%M)");
	u8 null_mac_address[6]={0,0,0,0,0,0};
	u8 test_mac_address[6]={0x11,0x22,0x33,0xaa,0xbb,0xcc};
	_test_format("00:00:00:00:00:00,11:22:33:aa:bb:cc","%M,%M",null_mac_address,test_mac_address);
	TEST_GROUP("IPv4 address (%I)");
	_test_format("0.0.0.0,11.22.33.255","%I,%I",0x00000000,0x0b1621ff);
	TEST_GROUP("unknown escape sequence");
	_test_format("%%%q%!","%%%q%!");
}
