#ifndef _NVME_REGISTERS_H_
#define _NVME_REGISTERS_H_ 1
#include <kernel/types.h>



// Capability flags
#define CAP_CSS_NVME 0x2000000000ull

// Controller Configuration flags
#define CC_EN 0x01

// Controller Status flags
#define CSTS_RDY 0x01

#define SQE_OPC_ADMIN_CREATE_IO_SQ 1
#define SQE_OPC_ADMIN_CREATE_IO_CQ 5
#define SQE_OPC_ADMIN_IDENTIFY 6

#define ADMIN_IDENTIFY_CNS_ID_NS 0
#define ADMIN_IDENTIFY_CNS_ID_CTRL 1



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



typedef volatile struct _NVME_COMPLETION_QUEUE_ENTRY{
	u32 cdw0;
	u8 _padding[4];
	u16 sq_head;
	u16 sq_id;
	u16 cid;
	u16 status;
} nvme_completion_queue_entry_t;



typedef volatile struct _NVME_SUBMISSION_QUEUE_ENTRY{
	u32 cdw0;
	u32 nsid;
	u8 _padding[8];
	u64 mptr;
	u64 dptr_prp1;
	u64 dptr_prp2;
	u32 extra_data[6];
} nvme_submission_queue_entry_t;



typedef volatile union _NVME_IDENTIFY_DATA{
	struct{
		u16 vid;
		u16 ssvid;
		char sn[20];
		char mn[40];
		char fr[8];
		u8 rab;
		u8 ieee[3];
		u8 cmic;
		u8 mdts;
		u8 _padding[438];
		u32 nn;
	} controller;
	struct{
		u64 nsze;
		u64 ncap;
		u64 nuse;
		u8 nsfeat;
		u8 nlbaf;
		u8 flbas;
		u8 _padding[101];
		struct{
			u16 ms;
			u8 lbads;
			u8 rp;
		} lbaf[16];
	} namespace;
} nvme_identify_data_t;



#endif
