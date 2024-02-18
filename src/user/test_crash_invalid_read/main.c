#include <sys/syscall/syscall.h>
#include <sys/types.h>



void main(void){
	(void)(*((volatile const u8*)NULL));
	_sys_syscall1(sys_syscall_get_table_offset("test_crash")|0x00000001,0x01);
}
