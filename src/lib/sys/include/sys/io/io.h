#ifndef _SYS_IO_IO_H_
#define _SYS_IO_IO_H_ 1
#include <sys/fd/fd.h>



extern sys_fd_t sys_io_input_fd;
extern sys_fd_t sys_io_output_fd;
extern sys_fd_t sys_io_error_fd;



void __sys_io_init(void);



u32 sys_io_print(const char* template,...);



s32 sys_io_input(_Bool blocking);



#endif
