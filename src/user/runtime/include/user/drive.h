#ifndef _USER_DRIVE_H_
#define _USER_DRIVE_H_ 1
#include <user/types.h>



typedef struct _DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[48];
	char serial_number[48];
	char model_number[48];
	u64 block_count;
	u64 block_size;
} drive_t;



_Bool drive_get(u32 index,drive_t* out);



#endif
