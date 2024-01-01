#ifndef _SYS2_IO_IO_H_
#define _SYS2_IO_IO_H_ 1
#include <sys2/fd/fd.h>



extern sys2_fd_t sys2_io_input_fd;
extern sys2_fd_t sys2_io_output_fd;
extern sys2_fd_t sys2_io_error_fd;



void __sys2_io_init(void);



u32 sys2_io_print(const char* template,...);



s32 sys2_io_input(_Bool blocking);



#endif
