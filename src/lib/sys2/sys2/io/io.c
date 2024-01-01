#include <sys2/fd/fd.h>
#include <sys2/types.h>



SYS2_PUBLIC sys2_fd_t sys2_io_input_handle;
SYS2_PUBLIC sys2_fd_t sys2_io_output_handle;
SYS2_PUBLIC sys2_fd_t sys2_io_error_handle;



static void SYS2_CONSTRUCTOR _init(void){
	sys2_io_input_handle=sys2_fd_open(0,"/proc/self/stdin",SYS2_FD_FLAG_READ);
	sys2_io_output_handle=sys2_fd_open(0,"/proc/self/stdout",SYS2_FD_FLAG_WRITE);
	sys2_io_error_handle=sys2_fd_open(0,"/proc/self/stderr",SYS2_FD_FLAG_WRITE);
}
