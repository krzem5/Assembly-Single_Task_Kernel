#ifndef _KERNEL_KVM_KVM_H_
#define _KERNEL_KVM_KVM_H_ 1
#include <kernel/types.h>



#define KVM_FEATURE_CLOCKSOURCE 0
#define KVM_FEATURE_NOP_IO_DELAY 1
#define KVM_FEATURE_MMU_OP 2
#define KVM_FEATURE_CLOCKSOURCE2 3
#define KVM_FEATURE_ASYNC_PF 4
#define KVM_FEATURE_STEAL_TIME 5
#define KVM_FEATURE_PV_EOI 6
#define KVM_FEATURE_PV_UNHALT 7
#define KVM_FEATURE_PV_TLB_FLUSH 9
#define KVM_FEATURE_ASYNC_PF_VMEXIT 10
#define KVM_FEATURE_PV_SEND_IPI 11
#define KVM_FEATURE_POLL_CONTROL 12
#define KVM_FEATURE_PV_SCHED_YIELD 13
#define KVM_FEATURE_ASYNC_PF_INT 14
#define KVM_FEATURE_MSI_EXT_DEST_ID 15
#define KVM_FEATURE_HC_MAP_GPA_RANGE 16
#define KVM_FEATURE_MIGRATION_CONTROL 17
#define KVM_FEATURE_CLOCKSOURCE_STABLE_BIT 24

#define KVM_MSR_WALL_CLOCK_NEW 0x4b564d00
#define KVM_MSR_SYSTEM_TIME_NEW 0x4b564d01
#define KVM_MSR_ASYNC_PF_EN 0x4b564d02
#define KVM_MSR_STEAL_TIME 0x4b564d03
#define KVM_MSR_PV_EOI_EN 0x4b564d04
#define KVM_MSR_POLL_CONTROL 0x4b564d05
#define KVM_MSR_ASYNC_PF_INT 0x4b564d06
#define KVM_MSR_ASYNC_PF_ACK 0x4b564d07
#define KVM_MSR_MIGRATION_CONTROL 0x4b564d08

#define KVM_MSR_FLAG_ENABLED 1



typedef volatile struct KERNEL_PACKED _KVM_TIME_INFO{
	u32 version;
	u8 _padding[4];
	u64 tsc_timestamp;
	u64 system_time;
	u32 tsc_to_system_mul;
	s8 tsc_shift;
	u8 flags;
	u8 _padding1[2];
} kvm_time_info_t;



_Bool _kvm_get_flags(u32* features,u32* feature_hints);



#endif
