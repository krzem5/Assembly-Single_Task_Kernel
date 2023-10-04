#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/sandbox/sandbox.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "sandbox"



PMM_DECLARE_COUNTER(OMM_SANDBOX);



static omm_allocator_t _sandbox_allocator=OMM_ALLOCATOR_INIT_LATER_STRUCT;
static sandbox_flag_t _sandbox_flag_count;



void sandbox_init(void){
	LOG("Initializing sandbox flags...");
	_sandbox_flag_count=0;
	for (const sandbox_descriptor_t*const* descriptor=(void*)kernel_section_sandbox_start();(u64)descriptor<kernel_section_sandbox_end();descriptor++){
		if (*descriptor){
			*((*descriptor)->var)=_sandbox_flag_count;
			_sandbox_flag_count++;
		}
	}
	INFO("Sandbox flag count: %u",_sandbox_flag_count);
	_sandbox_allocator=OMM_ALLOCATOR_INIT_STRUCT("sandbox",sizeof(sandbox_t)+((_sandbox_flag_count+63)>>6)*sizeof(u64),8,1,PMM_COUNTER_OMM_SANDBOX);
}



sandbox_t* sandbox_new(void){
	sandbox_t* out=omm_alloc(&_sandbox_allocator);
	memset(out,0,sizeof(sandbox_t)+((_sandbox_flag_count+63)>>6)*sizeof(u64));
	return out;
}



void sandbox_delete(sandbox_t* sandbox){
	omm_dealloc(&_sandbox_allocator,sandbox);
}



_Bool sandbox_get(const sandbox_t* sandbox,sandbox_flag_t flag){
	if (flag>=_sandbox_flag_count){
		panic("Sandbox flag out of range");
	}
	return !!(sandbox->bitmap[flag>>6]&(1ull<<(flag&63)));
}



_Bool sandbox_set(sandbox_t* sandbox,sandbox_flag_t flag,_Bool state){
	if (flag>=_sandbox_flag_count){
		panic("Sandbox flag out of range");
	}
	u64 mask=1ull<<(flag&63);
	flag>>=6;
	_Bool out=!!(sandbox->bitmap[flag]&mask);
	if (out!=state){
		sandbox->bitmap[flag]^=mask;
	}
	return out;
}
