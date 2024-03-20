#ifndef _KERNEL_TPM_TPM_H_
#define _KERNEL_TPM_TPM_H_ 1
#include <kernel/tpm/commands.h>
#include <kernel/types.h>



#define TPM_FLAG_VERSION_2 1



typedef struct _TPM_DEVICE_COMMAND{
	u16 cc;
	u16 attributes;
} tpm_device_command_t;



typedef struct _TPM_BANK{
	u16 hash_alg;
	u16 digest_size;
} tpm_bank_t;



typedef struct _TPM{
	u32 flags;
	volatile u32* regs;
	tpm_command_t* command;
	u32 device_command_count;
	u32 bank_count;
	tpm_device_command_t* device_commands;
	tpm_bank_t* banks;
} tpm_t;



#endif
