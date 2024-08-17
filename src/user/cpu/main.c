#include <cpuid.h>
#include <sys/clock/clock.h>
#include <sys/cpu/cpu.h>
#include <sys/io/io.h>
#include <sys/types.h>
#include <sys/util/options.h>



#define FEATURE(reg,bit,name) \
	if (cpuid.reg&(1<<bit)){ \
		sys_io_print(" %s",name); \
	}



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



int main(int argc,const char** argv){
	if (!sys_options_parse_NEW(argc,argv,"")){
		return 1;
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
	sys_io_print("CPU count: \x1b[1m%u\x1b[0m\nCPU frequency: \x1b[1m%lu MHz\x1b[0m\nCPU signature: \x1b[1m%s\x1b[0m\nCPU basic/extended feature level: \x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m\n",sys_cpu_get_count(),(sys_clock_get_frequency()+500000)/1000000,signature,max_level,max_extended_level);
	if (max_level>=1){
		_execute_cpuid(1,&cpuid);
		sys_io_print("CPU model/family/stepping: \x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m\n",(cpuid.eax>>8)&15,(cpuid.eax>>4)&15,cpuid.eax&15);
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
		sys_io_print("CPU model name: \x1b[1m%s\x1b[0m\n",brand_string);
	}
	if (max_level>=4){
		u32 count=0;
		_execute_cpuid_count(4,0,&cpuid);
		while (cpuid.eax||cpuid.ecx){
			sys_io_print("CPU cache#%u size: \x1b[1m%v\x1b[0m\n",count,(((cpuid.eax>>22)&0x3ff)+1)*(((cpuid.eax>>12)&0x3ff)+1)*((cpuid.eax&0xfff)+1)*(cpuid.ecx+1));
			count++;
			_execute_cpuid_count(4,count,&cpuid);
		}
	}
	if (max_extended_level>=8){
		_execute_cpuid(0x80000008,&cpuid);
		sys_io_print("CPU physical/virtual address size: \x1b[1m%u\x1b[0m/\x1b[1m%u\x1b[0m bits\n",cpuid.eax&0xff,(cpuid.eax>>8)&0xff);
	}
	sys_io_print("CPU flags:\x1b[1m");
	if (max_level>=1){
		_execute_cpuid(1,&cpuid);
		FEATURE(ecx,0,"sse3");
		FEATURE(ecx,1,"pclmulqdq");
		FEATURE(ecx,2,"dtes64");
		FEATURE(ecx,3,"monitor");
		FEATURE(ecx,4,"ds_cpl");
		FEATURE(ecx,5,"vmx");
		FEATURE(ecx,6,"smx");
		FEATURE(ecx,7,"eist");
		FEATURE(ecx,8,"tm2");
		FEATURE(ecx,9,"ssse3");
		FEATURE(ecx,10,"cnxt_id");
		FEATURE(ecx,11,"sdbg");
		FEATURE(ecx,12,"fma");
		FEATURE(ecx,13,"cmpxchg16b");
		FEATURE(ecx,14,"xtpr");
		FEATURE(ecx,15,"pdcm");
		FEATURE(ecx,17,"pcid");
		FEATURE(ecx,18,"dca");
		FEATURE(ecx,19,"sse4_1");
		FEATURE(ecx,20,"sse4_2");
		FEATURE(ecx,21,"x2apic");
		FEATURE(ecx,22,"movbe");
		FEATURE(ecx,23,"popcnt");
		FEATURE(ecx,24,"tsc_deadline_timer");
		FEATURE(ecx,25,"aesni");
		FEATURE(ecx,26,"xsave");
		FEATURE(ecx,27,"osxsave");
		FEATURE(ecx,28,"avx");
		FEATURE(ecx,29,"f16c");
		FEATURE(ecx,30,"rdrand");
		FEATURE(edx,0,"fpu");
		FEATURE(edx,1,"vme");
		FEATURE(edx,2,"de");
		FEATURE(edx,3,"pse");
		FEATURE(edx,4,"tsc");
		FEATURE(edx,5,"msr");
		FEATURE(edx,6,"pae");
		FEATURE(edx,7,"mce");
		FEATURE(edx,8,"cx8");
		FEATURE(edx,9,"apic");
		FEATURE(edx,11,"sep");
		FEATURE(edx,12,"mtrr");
		FEATURE(edx,13,"pge");
		FEATURE(edx,14,"mca");
		FEATURE(edx,15,"cmov");
		FEATURE(edx,16,"pat");
		FEATURE(edx,17,"pse36");
		FEATURE(edx,18,"psn");
		FEATURE(edx,19,"clfsh");
		FEATURE(edx,21,"ds");
		FEATURE(edx,22,"acpi");
		FEATURE(edx,23,"mmx");
		FEATURE(edx,24,"fxsr");
		FEATURE(edx,25,"sse");
		FEATURE(edx,26,"sse2");
		FEATURE(edx,27,"ss");
		FEATURE(edx,28,"htt");
		FEATURE(edx,29,"tm");
		FEATURE(edx,31,"pbe");
	}
	if (max_level>=7){
		_execute_cpuid_count(7,0,&cpuid);
		FEATURE(ebx,0,"fsgsbase");
		FEATURE(ebx,1,"ia32_tsc_adjust_msr");
		FEATURE(ebx,2,"sgx");
		FEATURE(ebx,3,"bmi1");
		FEATURE(ebx,4,"hle");
		FEATURE(ebx,5,"avx2");
		FEATURE(ebx,6,"fdp_excptn_only");
		FEATURE(ebx,7,"smep");
		FEATURE(ebx,8,"bmi2");
		FEATURE(ebx,9,"enhanced_rep_mov");
		FEATURE(ebx,10,"invpcid");
		FEATURE(ebx,11,"rtm");
		FEATURE(ebx,12,"rdt_m");
		FEATURE(ebx,13,"deprecated_fpu_cs_ds");
		FEATURE(ebx,14,"mpx");
		FEATURE(ebx,15,"rdt_a");
		FEATURE(ebx,16,"avx512f");
		FEATURE(ebx,17,"avx512dq");
		FEATURE(ebx,18,"rdseed");
		FEATURE(ebx,19,"adx");
		FEATURE(ebx,20,"smap");
		FEATURE(ebx,21,"avx512_ifma");
		FEATURE(ebx,23,"clflushopt");
		FEATURE(ebx,24,"clwb");
		FEATURE(ebx,25,"ptrace");
		FEATURE(ebx,26,"avx512pf");
		FEATURE(ebx,27,"avx512er");
		FEATURE(ebx,28,"avx512cd");
		FEATURE(ebx,29,"sha");
		FEATURE(ebx,30,"avx512bw");
		FEATURE(ebx,31,"avx512vl");
		FEATURE(ecx,0,"prefetchwt1");
		FEATURE(ecx,1,"avx512_vbmi");
		FEATURE(ecx,2,"umip");
		FEATURE(ecx,3,"pku");
		FEATURE(ecx,4,"ospke");
		FEATURE(ecx,5,"waitpkg");
		FEATURE(ecx,6,"avx512_vbmi2");
		FEATURE(ecx,7,"cet_ss");
		FEATURE(ecx,8,"gfni");
		FEATURE(ecx,9,"vaes");
		FEATURE(ecx,10,"vpclmulqdq");
		FEATURE(ecx,11,"avx512_vnni");
		FEATURE(ecx,12,"avx512_bitalg");
		FEATURE(ecx,14,"avx512_vpopcntdq");
		FEATURE(ecx,16,"la57");
		FEATURE(ecx,22,"rdpid");
		FEATURE(ecx,23,"kl");
		FEATURE(ecx,24,"bus_lock_detect");
		FEATURE(ecx,25,"cldemote");
		FEATURE(ecx,27,"movdiri");
		FEATURE(ecx,28,"movdir64b");
		FEATURE(ecx,29,"enqcmd");
		FEATURE(ecx,30,"sgx_lc");
		FEATURE(ecx,31,"pks");
		FEATURE(edx,1,"sgx_keys");
		FEATURE(edx,2,"avx512_4vnniw");
		FEATURE(edx,3,"avx512_4fmaps");
		FEATURE(edx,4,"fast_rep_mov");
		FEATURE(edx,5,"uintr");
		FEATURE(edx,8,"avx512_vp2intersect");
		FEATURE(edx,9,"srbds_ctrl");
		FEATURE(edx,10,"md_clear");
		FEATURE(edx,11,"rtm_always_abort");
		FEATURE(edx,13,"rtm_force_abort");
		FEATURE(edx,14,"serialize");
		FEATURE(edx,15,"hybrid");
		FEATURE(edx,16,"tsxldtrk");
		FEATURE(edx,18,"pconfig");
		FEATURE(edx,19,"architectural_lbr");
		FEATURE(edx,20,"cet_ibt");
		FEATURE(edx,22,"amx_bf16");
		FEATURE(edx,23,"avx512_fp16");
		FEATURE(edx,24,"amx_tile");
		FEATURE(edx,25,"amx_int8");
		FEATURE(edx,26,"ibrs");
		FEATURE(edx,27,"stibp");
		FEATURE(edx,28,"l1d_flush");
		FEATURE(edx,29,"arch_capabilities");
		FEATURE(edx,30,"core_capabilities");
		_execute_cpuid_count(7,1,&cpuid);
		FEATURE(eax,4,"avx_vnni");
		FEATURE(eax,5,"avx512_bf16");
		FEATURE(eax,10,"fast_zero_rep_movsb");
		FEATURE(eax,11,"fast_short_rep_stosb");
		FEATURE(eax,12,"fast_short_rep_cmpsb");
		FEATURE(eax,22,"hreset");
		FEATURE(edx,18,"cet_sss");
		_execute_cpuid_count(7,2,&cpuid);
		FEATURE(edx,0,"psfd");
		FEATURE(edx,1,"ipred_ctrl");
		FEATURE(edx,2,"rrsba_ctrl");
		FEATURE(edx,3,"ddpd_u");
		FEATURE(edx,4,"bhi_ctrl");
		FEATURE(edx,5,"mcdt_no");
	}
	if (max_extended_level>=1){
		_execute_cpuid(0x80000001,&cpuid);
		FEATURE(ecx,0,"lahf_sahf");
		FEATURE(ecx,5,"lzcnt");
		FEATURE(ecx,8,"prefetchw");
		FEATURE(edx,11,"syscall");
		FEATURE(edx,20,"nx");
		FEATURE(edx,26,"pg1gb");
		FEATURE(edx,27,"rdtscp");
		FEATURE(edx,29,"intel64");
	}
	if (max_extended_level>=8){
		_execute_cpuid(0x80000008,&cpuid);
		FEATURE(ebx,8,"wbnoinvd");
	}
	sys_io_print("\x1b[0m\n");
	return 0;
}
