#ifndef _SYS_PIPE_PIPE_H_
#define _SYS_PIPE_PIPE_H_ 1
#include <sys/error/error.h>
#include <sys/fd/fd.h>
#include <sys/types.h>



sys_error_t __attribute__((access(read_only,1))) sys_pipe_create(const void* path);



sys_error_t sys_pipe_close(sys_fd_t pipe);



#endif
