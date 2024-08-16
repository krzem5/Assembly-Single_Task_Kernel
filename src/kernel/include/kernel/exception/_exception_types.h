#ifndef _KERNEL_EXCEPTION__EXCEPTION_TYPES_H_
#define _KERNEL_EXCEPTION__EXCEPTION_TYPES_H_ 1
#include <kernel/types.h>



typedef struct _EXCEPTION_UNWIND_FRAME{
	struct _EXCEPTION_UNWIND_FRAME* next;
	void**const* args;
	void (*callback)(void**const*);
} exception_unwind_frame_t;



#endif
