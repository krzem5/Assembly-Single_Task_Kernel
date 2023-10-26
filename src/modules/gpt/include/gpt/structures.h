#ifndef _GPT_STRUCTURES_H_
#define _GPT_STRUCTURES_H_ 1
#include <kernel/types.h>



#define GPT_TABLE_HEADER_SIGNATURE 0x5452415020494645ull



typedef struct _GPT_TABLE_HEADER{
	u64 signature;
	u32 revision;
	u32 size;
	u8 _padding[24];
	u64 first_block;
	u64 last_block;
	u8 guid[16];
	u64 entry_lba;
	u32 entry_count;
	u32 entry_size;
} gpt_table_header_t;



#endif
