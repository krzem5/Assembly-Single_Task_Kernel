#ifndef _DIRCOLOR_DIRCOLOR_H_
#define _DIRCOLOR_DIRCOLOR_H_ 1
#include <sys/fd.h>



void dircolor_init(void);



void dircolor_get_color(const sys_fd_stat_t* stat,char* buffer);



void dircolor_get_color_with_link(const sys_fd_stat_t* stat,const char* name,s64 fd);



#endif
