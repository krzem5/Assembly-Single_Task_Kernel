#ifndef _KERNEL_LOG_LOG_H_
#define _KERNEL_LOG_LOG_H_ 1
#include <kernel/types.h>



#define LOG_TYPE_INFO 0
#define LOG_TYPE_LOG 1
#define LOG_TYPE_WARN 2
#define LOG_TYPE_ERROR 3

#define INFO(template,...) log(LOG_TYPE_INFO,KERNEL_LOG_NAME,template,##__VA_ARGS__)
#define LOG(template,...) log(LOG_TYPE_LOG,KERNEL_LOG_NAME,template,##__VA_ARGS__)
#define WARN(template,...) log(LOG_TYPE_WARN,KERNEL_LOG_NAME,template,##__VA_ARGS__)
#define ERROR(template,...) log(LOG_TYPE_ERROR,KERNEL_LOG_NAME,template,##__VA_ARGS__)



void log(u32 type,const char* name,const char* template,...);



void log_direct(const char* template,...);



#endif
