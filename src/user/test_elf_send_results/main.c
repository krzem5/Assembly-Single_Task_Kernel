#include <sys/io/io.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>



void main(u32 argc,const char*const argv,const char*const* environ,const u64* auxv){
	sys_io_print("ELF_TEST_RESULTS\n");
}
