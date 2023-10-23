#ifndef _ISO9660_STRUCTURES_H_
#define _ISO9660_STRUCTURES_H_ 1
#include <kernel/types.h>



// ISO 9660 directory flags
#define ISO9660_DIRECTORY_FLAG_HIDDEN 1
#define ISO9660_DIRECTORY_FLAG_DIRECTOR 2
#define ISO9660_DIRECTORY_FLAG_ASSOCIATED_FILE 4



typedef struct __attribute__((packed)) _ISO9660_VOLUME_DESCRIPTOR{
	u8 type;
	u8 identifier[5];
	u8 version;
	struct __attribute__((packed)){
		u8 _padding[33];
		char volume_name[32];
		u8 _padding2[8];
		u32 volume_size;
		u8 _padding3[74];
		u32 directory_lba;
		u8 _padding4[4];
		u32 directory_data_length;
	} primary_volume_descriptor;
} iso9660_volume_descriptor_t;



typedef struct __attribute__((packed)) _ISO9660_DIRECTORY{
	u8 length;
	u8 _padding;
	u32 lba;
	u8 _padding2[4];
	u32 data_length;
	u8 _padding3[11];
	u8 flags;
	u8 file_unit_size;
	u8 gap_size;
	u8 _padding4[4];
	u8 identifier_length;
	char identifier[];
} iso9660_directory_t;



#endif
