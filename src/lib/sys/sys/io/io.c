#include <sys/fd/fd.h>
#include <sys/format/format.h>
#include <sys/types.h>
#include <sys/util/var_arg.h>



SYS_PUBLIC sys_fd_t sys_io_input_fd=0;
SYS_PUBLIC sys_fd_t sys_io_output_fd=0;
SYS_PUBLIC sys_fd_t sys_io_error_fd=0;



void __sys_io_init(void){
	sys_io_input_fd=sys_fd_dup(SYS_FD_DUP_STDIN,SYS_FD_FLAG_READ);
	sys_io_output_fd=sys_fd_dup(SYS_FD_DUP_STDOUT,SYS_FD_FLAG_WRITE);
	sys_io_error_fd=sys_fd_dup(SYS_FD_DUP_STDERR,SYS_FD_FLAG_WRITE);
}



SYS_PUBLIC u32 sys_io_print(const char* template,...){
	sys_var_arg_list_t va;
	sys_var_arg_init(va,template);
	char buffer[4096];
	u32 out=sys_format_string_va(buffer,sizeof(buffer)/sizeof(char),template,&va);
	sys_var_arg_deinit(va);
	return sys_fd_write(sys_io_output_fd,buffer,out,0);
}



SYS_PUBLIC s32 sys_io_input(bool blocking){
	char buffer[1];
	return (sys_fd_read(sys_io_input_fd,buffer,1,(blocking?0:SYS_FD_FLAG_NONBLOCKING))==1?buffer[0]:-1);
}
