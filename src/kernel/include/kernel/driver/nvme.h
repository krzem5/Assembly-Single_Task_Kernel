#ifndef _KERNEL_DRIVER_NVME_H_
#define _KERNEL_DRIVER_NVME_H_
#include <kernel/types.h>



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



void driver_nvme_init(void);



#endif
