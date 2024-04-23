#include <glsl/_internal/interface_allocator.h>
#include <sys/heap/heap.h>
#include <sys/types.h>



void _glsl_interface_allocator_init(u32 size,glsl_interface_allocator_t* out){
	out->bitmap=sys_heap_alloc(NULL,((size+32+31)>>5)*sizeof(u32));
	for (u32 i=0;i<=(size>>5);i++){
		out->bitmap[i]=0;
	}
	out->size=size;
}



void _glsl_interface_allocator_deinit(glsl_interface_allocator_t* allocator){
	sys_heap_dealloc(NULL,allocator->bitmap);
	allocator->bitmap=NULL;
	allocator->size=0;
}



bool _glsl_interface_allocator_reserve(glsl_interface_allocator_t* allocator,u32* offset,u32 size,u32 align){
	if (*offset!=0xffffffff){
		u32 i=*offset;
		if (i>=allocator->size||i+size>=allocator->size){
			return 0;
		}
		for (size+=i;i<size;i++){
			if (allocator->bitmap[i>>5]&(1<<(i&31))){
				return 0;
			}
			allocator->bitmap[i>>5]|=1<<(i&31);
		}
		return 1;
	}
	u64 base_mask=(1<<size)-1;
	for (u32 i=0;i<=allocator->size-size;i+=align){
		u64 shifted_mask=base_mask<<(i&31);
		if (!(allocator->bitmap[i>>5]&shifted_mask)&&!(allocator->bitmap[(i>>5)+1]&(shifted_mask>>32))){
			*offset=i;
			allocator->bitmap[i>>5]|=shifted_mask;
			allocator->bitmap[(i>>5)+1]|=shifted_mask>>32;
			return 1;
		}
	}
	return 0;
}



void _glsl_interface_allocator_release(glsl_interface_allocator_t* allocator,u32 offset,u32 size){
	for (size+=offset;offset<size;offset++){
		allocator->bitmap[offset>>5]&=~(1<<(offset&31));
	}
}
