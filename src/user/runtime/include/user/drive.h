#ifndef _USER_DRIVE_H_
#define _USER_DRIVE_H_ 1
#include <user/types.h>



typedef struct _DRIVE_DATA{
	char name[64];
	char serial_number[64];
	char model_number[64];
	char type[64];
	u64 block_count;
	u64 block_size;
} drive_data_t;



_Bool drive_get_data(u64 handle,drive_data_t* out);



#endif
