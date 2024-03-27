#ifndef _KERNEL_UTIL_SPINLOOP_H_
#define _KERNEL_UTIL_SPINLOOP_H_ 1



#ifdef KERNEL_COVERAGE
#define SPINLOOP(cond) \
	do{ \
		KERNEL_INLINE void KERNEL_NOCOVERAGE __nocoverage_spinloop(void){ \
			while (cond){ \
				__pause(); \
			} \
		} \
		__nocoverage_spinloop(); \
	} while(0)
#define COUNTER_SPINLOOP(max) \
	do{ \
		KERNEL_INLINE void KERNEL_NOCOVERAGE __nocoverage_counter_spinloop(void){ \
			for (u64 __tmp=0;__tmp<(max);__tmp++){ \
				__pause(); \
			} \
		} \
		__nocoverage_counter_spinloop(); \
	} while(0)
#else
#define SPINLOOP(cond) \
	while (cond){ \
		__pause(); \
	}
#define COUNTER_SPINLOOP(max) \
	for (u64 __tmp=0;__tmp<(max);__tmp++){ \
		__pause(); \
	}
#endif



static KERNEL_INLINE void KERNEL_NOCOVERAGE __pause(void){
	asm volatile("pause":::"memory");
}



#endif
