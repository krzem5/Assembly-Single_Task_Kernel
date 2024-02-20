#include <sys/format/format.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <sys/util/var_arg.h>
#include <test/test.h>



#define TEST_FORMAT_BUFFER_SIZE 128



static void _test_format_empty(const char* marker,...){
	char buffer[TEST_FORMAT_BUFFER_SIZE];
	sys_var_arg_list_t va;
	sys_var_arg_init(va,marker);
	TEST_ASSERT(!sys_format_string_va(buffer,0,"",&va));
	sys_var_arg_deinit(va);
}



static void _test_format(const char* expected,const char* template,...){
	char buffer[TEST_FORMAT_BUFFER_SIZE];
	sys_var_arg_list_t va;
	sys_var_arg_init(va,template);
	sys_format_string_va(buffer,TEST_FORMAT_BUFFER_SIZE,template,&va);
	sys_var_arg_deinit(va);
	TEST_ASSERT(!sys_string_compare(buffer,expected));
}



void test_sys_format(void){
	TEST_MODULE("sys_format");
	TEST_FUNC("sys_format_string_va");
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
	TEST_GROUP("wide hexadecimal (%w)");
	_test_format("00000000,11223344","%w,%w",0,0x11223344);
	TEST_GROUP("guid (%g)");
	u8 null_guid[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	u8 test_guid[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
	_test_format("00000000-0000-0000-0000-000000000000,00112233-4455-6677-8899-aabbccddeeff","%g,%g",null_guid,test_guid);
	TEST_GROUP("MAC address (%M)");
	u8 null_mac_address[6]={0,0,0,0,0,0};
	u8 test_mac_address[6]={0x11,0x22,0x33,0xaa,0xbb,0xcc};
	_test_format("00:00:00:00:00:00,11:22:33:aa:bb:cc","%M,%M",null_mac_address,test_mac_address);
	TEST_GROUP("IPv4 address (%I)");
	_test_format("0.0.0.0,11.22.33.255","%I,%I",0x00000000,0x0b1621ff);
	TEST_GROUP("time (%t)");
	_test_format("1970-01-01 00:00:00.000000000,2024-02-20 20:15:44.530905628","%t,%t",0,1708460144530905628ull);
	TEST_GROUP("unknown escape sequence");
	_test_format("%%%q%!","%%%q%!");
}
