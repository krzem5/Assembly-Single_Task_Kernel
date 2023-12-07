#ifndef _KERNEL_CPU_LOCAL_H_
#define _KERNEL_CPU_LOCAL_H_ 1
#include <kernel/cpu/cpu.h>
#include <kernel/types.h>



#define CPU_LOCAL(name) ((name)+CPU_HEADER_DATA->index)

#define CPU_LOCAL_DATA(type,name) \
	KERNEL_INIT_WRITE type* name=NULL; \
	static const cpu_local_data_descriptor_t KERNEL_EARLY_READ _cpu_local_data_descriptor_##name={ \
		(void**)(&name), \
		sizeof(type) \
	}; \
	static const cpu_local_data_descriptor_t*const __attribute__((used,section(".cpulocal"))) _cpu_local_data_descriptor_ptr_##name=&_cpu_local_data_descriptor_##name;



typedef struct _CPU_LOCAL_DATA_DESCRIPTOR{
	void** var;
	u32 size;
} cpu_local_data_descriptor_t;



void cpu_local_init(void);



#endif
