#ifndef _CWD_H_
#define _CWD_H_ 1
#include <sys/fd/fd.h>
#include <sys/types.h>



extern char cwd[];
extern u32 cwd_length;
extern sys_fd_t cwd_fd;



void cwd_init(void);



_Bool cwd_change(const char* path);



#endif
