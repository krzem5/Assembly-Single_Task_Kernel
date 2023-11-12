#ifndef _GPT_STRUCTURES_H_
#define _GPT_STRUCTURES_H_ 1
#include <kernel/types.h>



#define GPT_TABLE_HEADER_SIGNATURE 0x5452415020494645ull



typedef struct _GPT_TABLE_HEADER{
	u64 signature;
	u32 revision;
	u32 size;
	u8 _padding[24];
	u64 first_lba;
	u64 last_lba;
	u8 guid[16];
	u64 entry_lba;
	u32 entry_count;
	u32 entry_size;
} gpt_table_header_t;



typedef struct _GPT_PARTITION_ENTRY{
	u8 type_guid[16];
	u8 guid[16];
	u64 start_lba;
	u64 end_lba;
	u64 attributes;
	u16 name[36];
} gpt_partition_entry_t;



#endif
