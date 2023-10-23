#ifndef _NVME_REGISTERS_H_
#define _NVME_REGISTERS_H_ 1
#include <kernel/types.h>



// Controller Configuration flags
#define CC_EN 0x01

// Controller Status flags
#define CSTS_RDY 0x01



typedef volatile struct _NVME_REGISTERS{
	u64 cap;
	u32 vs;
	u32 intms;
	u32 intmc;
	u32 cc;
	u32 rsvd1;
	u32 csts;
	u32 rsvd2;
	u32 aqa;
	u64 asq;
	u64 acq;
} nvme_registers_t;



#endif
