#include <kernel/clock/clock.h>
#include <kernel/kvm/kvm.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/time/time.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "kvm"



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kvm_pmm_counter=NULL;
static u32 KERNEL_INIT_WRITE _kvm_features=0;
static u32 KERNEL_INIT_WRITE _kvm_feature_hints=0;
static bool KERNEL_INIT_WRITE _kvm_clock_source_is_stable=0;



static u64 KERNEL_EARLY_EXEC _calibration_callback(void){
	u32 msr_register_system_time=((_kvm_features&(1<<KVM_FEATURE_CLOCKSOURCE2))?KVM_MSR_SYSTEM_TIME_NEW:KVM_MSR_SYSTEM_TIME);
	u32 msr_register_wall_clock=((_kvm_features&(1<<KVM_FEATURE_CLOCKSOURCE2))?KVM_MSR_WALL_CLOCK_NEW:KVM_MSR_WALL_CLOCK);
	u64 physical_buffer=pmm_alloc(1,_kvm_pmm_counter,0);
	msr_write(msr_register_system_time,physical_buffer|KVM_MSR_FLAG_ENABLED);
	const kvm_time_info_t* time_info=(void*)(physical_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u64 value=(1000000000ull<<32)/time_info->tsc_to_system_mul;
	u64 out=(time_info->tsc_shift<0?value<<(-time_info->tsc_shift):value>>time_info->tsc_shift);
	u64 tsc_timestamp=time_info->tsc_timestamp;
	msr_write(msr_register_system_time,0);
	msr_write(msr_register_wall_clock,physical_buffer);
	const kvm_wall_clock_t* wall_clock=(void*)(physical_buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u32 version=0;!version||(wall_clock->version&1)||version!=wall_clock->version;){
		version=wall_clock->version;
		value=wall_clock->sec*1000000000ull+wall_clock->nsec;
	}
	time_boot_offset=value-clock_get_time()+clock_ticks_to_time(tsc_timestamp);
	INFO("Updating boot time offset to %lu",time_boot_offset);
	msr_write(msr_register_wall_clock,0);
	pmm_dealloc(physical_buffer,1,_kvm_pmm_counter);
	return out;
}



static const clock_source_t _kvm_tsc_clock_source={
	"TSC/KVM",
	_calibration_callback,
	&_kvm_clock_source_is_stable
};



KERNEL_EARLY_EARLY_INIT(){
	if (!_kvm_get_flags(&_kvm_features,&_kvm_feature_hints)){
		return;
	}
	_kvm_pmm_counter=pmm_alloc_counter("kvm");
	LOG("KVM hypervisor detected");
	if (_kvm_features&(1<<KVM_FEATURE_CLOCKSOURCE_STABLE_BIT)){
		_kvm_clock_source_is_stable=1;
	}
	if (_kvm_features&((1<<KVM_FEATURE_CLOCKSOURCE)|(1<<KVM_FEATURE_CLOCKSOURCE2))){
		clock_add_source(&_kvm_tsc_clock_source);
	}
}
