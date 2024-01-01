#include <sys2/fd/fd.h>
#include <sys2/format/format.h>
#include <sys2/types.h>
#include <sys2/util/var_arg.h>



SYS2_PUBLIC sys2_fd_t sys2_io_input_fd=0;
SYS2_PUBLIC sys2_fd_t sys2_io_output_fd=0;
SYS2_PUBLIC sys2_fd_t sys2_io_error_fd=0;



void __sys2_io_init(void){
	sys2_io_input_fd=sys2_fd_open(0,"/proc/self/stdin",SYS2_FD_FLAG_READ);
	sys2_io_output_fd=sys2_fd_open(0,"/proc/self/stdout",SYS2_FD_FLAG_WRITE);
	sys2_io_error_fd=sys2_fd_open(0,"/proc/self/stderr",SYS2_FD_FLAG_WRITE);
}



SYS2_PUBLIC u32 sys2_io_print(const char* template,...){
	sys2_var_arg_list_t va;
	sys2_var_arg_init(va,template);
	char buffer[4096];
	u32 out=sys2_format_string_va(buffer,sizeof(buffer)/sizeof(char),template,&va);
	sys2_var_arg_deinit(va);
	return sys2_fd_write(sys2_io_output_fd,buffer,out,0);
}



SYS2_PUBLIC s32 sys2_io_input(_Bool blocking){
	char buffer[1];
	return (sys2_fd_read(sys2_io_input_fd,buffer,1,(blocking?0:SYS2_FD_FLAG_NONBLOCKING))==1?buffer[0]:-1);
}
