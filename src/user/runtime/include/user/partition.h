#ifndef _USER_PARTITION_H_
#define _USER_PARTITION_H_ 1
#include <user/types.h>



typedef struct _PARTITION_DATA{
	char name[64];
	char partition_table_name[64];
	u64 drive_handle;
	u64 start_lba;
	u64 end_lba;
	u64 fs_handle;
} partition_data_t;



_Bool partition_get_data(u64 handle,partition_data_t* out);



#endif
