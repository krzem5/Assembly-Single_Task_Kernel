#ifndef _KERNEL_CONTEXT_CONTEXT_H_
#define _KERNEL_CONTEXT_CONTEXT_H_ 1



void context_load(void);



void context_save(void);



void _context_get_cpu_state(void* buffer);



#endif
