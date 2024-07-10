#ifndef _GPT_STRUCTURES_H_
#define _GPT_STRUCTURES_H_ 1
#include <kernel/types.h>



#define GPT_TABLE_HEADER_SIGNATURE 0x5452415020494645ull



typedef struct KERNEL_PACKED _GPT_TABLE_HEADER{
	u64 signature;
	u32 revision;
	u32 header_size;
	u32 crc;
	u32 _zero;
	u64 current_lba;
	u64 backup_lba;
	u64 first_lba;
	u64 last_lba;
	u8 guid[16];
	u64 entry_lba;
	u32 entry_count;
	u32 entry_size;
	u32 entry_crc;
} gpt_table_header_t;



typedef struct KERNEL_PACKED _GPT_PARTITION_ENTRY{
	u8 type_guid[16];
	u8 guid[16];
	u64 start_lba;
	u64 end_lba;
	u64 attributes;
	u16 name[36];
} gpt_partition_entry_t;



#endif
