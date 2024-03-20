#ifndef _KERNEL_TPM_TPM_H_
#define _KERNEL_TPM_TPM_H_ 1
#include <kernel/types.h>



#define TPM_FLAG_VERSION_2 1



typedef struct _TPM_COMMAND{
	u16 cc;
	u16 attributes;
} tpm_command_t;



typedef struct _TPM_BANK{
	u16 hash_alg;
	u16 digest_size;
} tpm_bank_t;



typedef struct _TPM{
	u32 flags;
	volatile u32* regs;
	void* command_buffer;
	u32 command_count;
	u32 bank_count;
	tpm_command_t* commands;
	tpm_bank_t* banks;
} tpm_t;



#endif
