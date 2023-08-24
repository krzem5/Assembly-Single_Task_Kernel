#include <command.h>
#include <cpuid.h>
#include <user/clock.h>
#include <user/cpu.h>
#include <user/io.h>



typedef struct _CPUID_DATA{
	u32 eax;
	u32 ebx;
	u32 ecx;
	u32 edx;
} cpuid_data_t;



static inline void _execute_cpuid(u32 level,cpuid_data_t* out){
	__cpuid(level,out->eax,out->ebx,out->ecx,out->edx);
}



static inline void _execute_cpuid_count(u32 level,u32 count,cpuid_data_t* out){
	__cpuid_count(level,count,out->eax,out->ebx,out->ecx,out->edx);
}



void cpu_main(int argc,const char*const* argv){
	if (argc>1){
		printf("cpu: unrecognized option '%s'\n",argv[1]);
		return;
	}
	cpuid_data_t cpuid;
	char signature[13];
	_execute_cpuid(0,&cpuid);
	*((u32*)signature)=cpuid.ebx;
	*((u32*)(signature+4))=cpuid.edx;
	*((u32*)(signature+8))=cpuid.ecx;
	signature[12]=0;
	u32 max_level=cpuid.eax;
	_execute_cpuid(0x80000000,&cpuid);
	u32 max_extended_level=cpuid.eax-0x80000000;
	printf("CPU count: \x1b[1m%u\x1b[0m\nCPU frequency: \x1b[1m%lu MHz\x1b[0m\nCPU signature: \x1b[1m%s\x1b[0m\nCPU basic/extended feature level: \x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m\n",cpu_count,(clock_cpu_frequency+500000)/1000000,signature,max_level,max_extended_level);
	if (max_level>=1){
		_execute_cpuid(1,&cpuid);
		printf("CPU model/family/stepping: \x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m\n",(cpuid.eax>>8)&15,(cpuid.eax>>4)&15,cpuid.eax&15);
	}
	if (max_extended_level>=4){
		char brand_string[49];
		_execute_cpuid(0x80000002,&cpuid);
		*((u32*)brand_string)=cpuid.eax;
		*((u32*)(brand_string+4))=cpuid.ebx;
		*((u32*)(brand_string+8))=cpuid.ecx;
		*((u32*)(brand_string+12))=cpuid.edx;
		_execute_cpuid(0x80000003,&cpuid);
		*((u32*)(brand_string+16))=cpuid.eax;
		*((u32*)(brand_string+20))=cpuid.ebx;
		*((u32*)(brand_string+24))=cpuid.ecx;
		*((u32*)(brand_string+28))=cpuid.edx;
		_execute_cpuid(0x80000004,&cpuid);
		*((u32*)(brand_string+32))=cpuid.eax;
		*((u32*)(brand_string+36))=cpuid.ebx;
		*((u32*)(brand_string+40))=cpuid.ecx;
		*((u32*)(brand_string+44))=cpuid.edx;
		brand_string[48]=0;
		printf("CPU model name: \x1b[1m%s\x1b[0m\n",brand_string);
	}
	if (max_level>=4){
		u32 count=0;
		_execute_cpuid_count(4,0,&cpuid);
		while (cpuid.eax||cpuid.ecx){
			printf("CPU cache#%u size: \x1b[1m%v\x1b[0m\n",count,(((cpuid.eax>>22)&0x3ff)+1)*(((cpuid.eax>>12)&0x3ff)+1)*((cpuid.eax&0xfff)+1)*(cpuid.ecx+1));
			count++;
			_execute_cpuid_count(4,count,&cpuid);
		}
	}
	if (max_extended_level>=8){
		_execute_cpuid(0x80000008,&cpuid);
		printf("CPU physical/virtual address size: \x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m bits\n",cpuid.eax&0xff,(cpuid.eax>>8)&0xff);
	}
}



DECLARE_COMMAND(cpu,"cpu");
