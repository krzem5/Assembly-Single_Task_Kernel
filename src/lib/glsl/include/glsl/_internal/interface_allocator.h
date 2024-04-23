#ifndef _GLSL__INTERNAL_INTERFACE_ALLOCATOR_H_
#define _GLSL__INTERNAL_INTERFACE_ALLOCATOR_H_ 1
#include <sys/types.h>



typedef struct _GLSL_INTERFACE_ALLOCATOR{
	u32* bitmap;
	u32 size;
} glsl_interface_allocator_t;



void _glsl_interface_allocator_init(u32 size,glsl_interface_allocator_t* out);



void _glsl_interface_allocator_deinit(glsl_interface_allocator_t* allocator);



bool _glsl_interface_allocator_reserve(glsl_interface_allocator_t* allocator,u32* offset,u32 size,u32 align);



void _glsl_interface_allocator_release(glsl_interface_allocator_t* allocator,u32 offset,u32 size);



#endif
