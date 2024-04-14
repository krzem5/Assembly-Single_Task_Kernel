#ifndef _NVME_DEVICE_H_
#define _NVME_DEVICE_H_ 1
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>
#include <nvme/registers.h>



typedef struct _NVME_QUEUE{
	volatile u32* doorbell;
	u16 mask;
} nvme_queue_t;



typedef struct _NVME_COMPLETION_QUEUE{
	nvme_queue_t queue;
	nvme_completion_queue_entry_t* entries;
	u16 head;
	_Bool phase;
} nvme_completion_queue_t;



typedef struct _NVME_SUBMISSION_QUEUE{
	nvme_queue_t queue;
	nvme_submission_queue_entry_t* entries;
	nvme_completion_queue_t* completion_queue;
	spinlock_t lock;
	u16 head;
	u16 tail;
} nvme_submission_queue_t;



typedef struct _NVME_DEVICE{
	nvme_registers_t* registers;
	u32 doorbell_stride;
	u32 max_request_size;
	nvme_completion_queue_t admin_completion_queue;
	nvme_submission_queue_t admin_submission_queue;
	nvme_completion_queue_t io_completion_queue;
	nvme_submission_queue_t io_submission_queue;
	u16 index;
} nvme_device_t;



#endif
