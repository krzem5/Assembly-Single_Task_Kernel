#ifndef _KERNEL_TPM_COMMANDS_H_
#define _KERNEL_TPM_COMMANDS_H_ 1
#include <kernel/types.h>



#define TPM2_PLATFORM_PCR_COUNT 24



#define TPM2_ST_NO_SESSIONS 0x8001
#define TPM2_ST_SESSIONS 0x8002

#define TPM2_RC_SUCCESS 0x0000
#define TPM2_RC_INITIALIZE 0x0100
#define TPM2_RC_TESTING 0x090a

#define TPM2_CC_CREATEPRIMARY 0x0131
#define TPM2_CC_SELF_TEST 0x0143
#define TPM2_CC_GET_CAPABILITY 0x017a
#define TPM2_CC_PCR_READ 0x017e

#define TPM_RH_OWNER 0x40000001

#define TPM2_CAP_COMMANDS 2
#define TPM2_CAP_PCRS 5
#define TPM2_CAP_TPM_PROPERTIES 6

#define TPM2_CC_FIRST 0x011f

#define TPM_PT_TOTAL_COMMANDS 0x0129

#define TPM_ALG_SHA1 0x0004
#define TPM_ALG_SHA256 0x000b
#define TPM_ALG_SHA384 0x000c
#define TPM_ALG_SHA512 0x000d



typedef struct KERNEL_PACKED _TPM_COMMAND_HEADER{
	u16 tag;
	u32 length;
	union{
		u32 command_code;
		u32 return_code;
	};
} tpm_command_header_t;



typedef struct KERNEL_PACKED _TPM_COMMAND{
	tpm_command_header_t header;
	union{
		u8 _raw_data[0];
		struct KERNEL_PACKED{
			u8 full_test;
		} self_test;
		struct KERNEL_PACKED{
			u32 capability;
			u32 property;
			u32 property_count;
		} get_capability;
		struct KERNEL_PACKED{
			u8 more_data;
			u32 property;
			u32 command_count;
			u32 commands[];
		} get_capability_resp_commands;
		struct KERNEL_PACKED{
			u8 more_data;
			u32 property;
			u32 bank_count;
			u8 data[];
		} get_capability_resp_pcrs;
		struct KERNEL_PACKED{
			u8 more_data;
			u32 capability;
			u32 property_count;
			u32 property;
			u32 value;
		} get_capability_resp_tpm_properties;
		struct KERNEL_PACKED{
			u32 selection_count;
			u16 selection_hash_alg;
			u8 selection_size;
			u8 selection_data[(TPM2_PLATFORM_PCR_COUNT+7)>>3];
		} pcr_read;
		struct KERNEL_PACKED{
			u32 update_counter_value;
			u32 selection_count;
			u16 selection_hash_alg;
			u8 selection_size;
			u8 selection_data[(TPM2_PLATFORM_PCR_COUNT+7)>>3];
			u32 digest_count;
			u16 digest_size;
			u8 data[];
		} pcr_read_resp;
	};
} tpm_command_t;



#endif
