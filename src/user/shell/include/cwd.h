#ifndef _CWD_H_
#define _CWD_H_ 1
#include <sys2/fd/fd.h>
#include <sys2/types.h>



extern char cwd[];
extern u32 cwd_length;
extern sys2_fd_t cwd_fd;



void cwd_init(void);



_Bool cwd_change(const char* path);



#endif
