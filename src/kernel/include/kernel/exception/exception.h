#ifndef _KERNEL_EXCEPTION_EXCEPTION_H_
#define _KERNEL_EXCEPTION_EXCEPTION_H_ 1
#include <kernel/types.h>



#define EXCEPTION_UNWIND_ARG(index) (*(__exception_args[(index)]))

#define _EXCEPTION_UNWIND_REF_ARG(arg) (void**)(&(arg))
#define _EXCEPTION_UNWIND_REF_ARGS_0(arg) NULL
#define _EXCEPTION_UNWIND_REF_ARGS_1(arg) _EXCEPTION_UNWIND_REF_ARG(arg)
#define _EXCEPTION_UNWIND_REF_ARGS_2(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_1(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_3(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_2(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_4(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_3(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_5(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_4(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_6(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_5(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_7(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_6(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_8(arg,...) _EXCEPTION_UNWIND_REF_ARG(arg),_EXCEPTION_UNWIND_REF_ARGS_7(__VA_ARGS__)
#define _EXCEPTION_UNWIND_REF_ARGS_(_1,_2,_3,_4,_5,_6,_7,_8,_9,N,...) _EXCEPTION_UNWIND_REF_ARGS_##N
#define _EXCEPTION_UNWIND_REF_ARGS(...) _EXCEPTION_UNWIND_REF_ARGS_(,##__VA_ARGS__,8,7,6,5,4,3,2,1,0,0)(__VA_ARGS__)

#define exception_unwind_push(...) \
	void**const __exception_unwind_args[]={_EXCEPTION_UNWIND_REF_ARGS(__VA_ARGS__)}; \
	auto void __exception_unwind_callback(void**const*); \
	exception_unwind_frame_t __exception_unwind_frame={ \
		__exception_unwind_args,\
		__exception_unwind_callback \
	}; \
	_exception_push_unwind_frame(&__exception_unwind_frame); \
	void __exception_unwind_callback(void**const* __exception_args)

#define exception_unwind_pop() \
	_exception_pop_unwind_frame(&__exception_unwind_frame);



typedef struct _EXCEPTION_UNWIND_FRAME{
	void**const* args;
	void (*callback)(void**const*);
	struct _EXCEPTION_UNWIND_FRAME* next;
} exception_unwind_frame_t;



void exception_unwind(void);



void _exception_push_unwind_frame(exception_unwind_frame_t* frame);



void _exception_pop_unwind_frame(exception_unwind_frame_t* frame);



void _exception_signal_interrupt_handler(u64 error);



KERNEL_NORETURN void _exception_user_return(u64 error);



#endif
