#ifndef _CWD_H_
#define _CWD_H_ 1
#include <user/types.h>



extern char cwd[];
extern u32 cwd_length;
extern int cwd_fd;



void cwd_init(void);



#endif
