#ifndef _KERNEL_LOG_LOG_H_
#define _KERNEL_LOG_LOG_H_ 1
#include <kernel/types.h>



#define LOG_TYPE_INFO 0
#define LOG_TYPE_LOG 1
#define LOG_TYPE_WARN 2
#define LOG_TYPE_ERROR 3
#define LOG_TYPE_INVALID 0xff

#define INFO(template,...) log(LOG_TYPE_INFO,KERNEL_LOG_NAME,template,##__VA_ARGS__)
#define LOG(template,...) log(LOG_TYPE_LOG,KERNEL_LOG_NAME,template,##__VA_ARGS__)
#define WARN(template,...) log(LOG_TYPE_WARN,KERNEL_LOG_NAME,template,##__VA_ARGS__)
#define ERROR(template,...) log(LOG_TYPE_ERROR,KERNEL_LOG_NAME,template,##__VA_ARGS__)



typedef struct _LOG_ENTRY{
	KERNEL_ATOMIC u8 type;
	u8 name_length;
	u16 data_length;
	char name_and_data[];
} log_entry_t;



typedef struct _LOG_BUFFER{
	struct _LOG_BUFFER* prev;
	struct _LOG_BUFFER* next;
	u32 offset;
	u32 size;
	void* ptr;
} log_buffer_t;



void log(u32 type,const char* name,const char* template,...);



void log_direct(const char* template,...);



void log_mask_type(u32 type);



void log_unmask_type(u32 type);



const log_buffer_t* log_get_buffer(void);



#endif
