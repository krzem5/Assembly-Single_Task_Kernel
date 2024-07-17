#ifndef _KERNEL_EXCEPTION_EXCEPTION_H_
#define _KERNEL_EXCEPTION_EXCEPTION_H_ 1
#include <kernel/types.h>



#define EXCEPTION_ARG(index) __exception_args[(index)]



#define exception_unwind_push(vars) \
	const void* __exception_unwind_args[1]=vars; \
	auto void __exception_unwind_callback(void**); \
	exception_unwind_frame_t __exception_unwind_frame={ \
		NULL, \
		__exception_unwind_args, \
		__exception_unwind_callback \
	}; \
	_exception_push_unwind_frame(&__exception_unwind_frame); \
	void __exception_unwind_callback(void** __exception_args)

#define exception_unwind_pop() \
	_exception_pop_unwind_frame(&__exception_unwind_frame);



typedef struct _EXCEPTION_UNWIND_FRAME{
	struct _EXCEPTION_UNWIND_FRAME* next;
	const void** args;
	void (*callback)(void**);
} exception_unwind_frame_t;



void _exception_push_unwind_frame(exception_unwind_frame_t* frame);



void _exception_pop_unwind_frame(exception_unwind_frame_t* frame);



#endif
