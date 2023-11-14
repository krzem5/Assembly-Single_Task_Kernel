#ifndef _CWD_H_
#define _CWD_H_ 1
#include <core/types.h>



extern char cwd[];
extern u32 cwd_length;
extern s64 cwd_fd;



void cwd_init(void);



_Bool cwd_change(const char* path);



#endif
