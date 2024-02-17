#include <sys/syscall/syscall.h>
#include <sys/types.h>



void main(u32 argc,const char*const* argv,const char*const* environ,const u64* auxv){
	_sys_syscall4(sys_syscall_get_table_offset("test_elf")|0x00000001,argc,(u64)argv,(u64)environ,(u64)auxv);
}
