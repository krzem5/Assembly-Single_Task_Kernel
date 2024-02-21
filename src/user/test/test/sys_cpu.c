#include <sys/cpu/cpu.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <test/test.h>



void test_sys_cpu(void){
	TEST_MODULE("sys_cpu");
	TEST_FUNC("sys_cpu_get_count");
	TEST_GROUP("correct args");
	TEST_ASSERT(sys_cpu_get_count()==_sys_syscall0(sys_syscall_get_table_offset("test_sys_cpu")|0x00000001));
}
