#include <kernel/clock/clock.h>
#include <kernel/kvm/kvm.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "kvm"



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kvm_pmm_counter=NULL;
static u32 KERNEL_INIT_WRITE _kvm_features=0;
static u32 KERNEL_INIT_WRITE _kvm_feature_hints=0;
static _Bool KERNEL_INIT_WRITE _kvm_clock_source_is_stable=0;



static u64 KERNEL_EARLY_EXEC _calibration_callback(void){
	u64 msr_register=((_kvm_features&(1<<KVM_FEATURE_CLOCKSOURCE2))?KVM_MSR_SYSTEM_TIME_NEW:KVM_MSR_SYSTEM_TIME);
	u64 physical_time_info=pmm_alloc(pmm_align_up_address(sizeof(kvm_time_info_t))>>PAGE_SIZE_SHIFT,_kvm_pmm_counter,0);
	msr_write(msr_register,physical_time_info|KVM_MSR_FLAG_ENABLED);
	kvm_time_info_t* info=(void*)(physical_time_info+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u64 value=(1000000000ull<<32)/info->tsc_to_system_mul;
	u64 out=(info->tsc_shift<0?value<<(-info->tsc_shift):value>>info->tsc_shift);
	msr_write(msr_register,0);
	pmm_dealloc(physical_time_info,pmm_align_up_address(sizeof(kvm_time_info_t))>>PAGE_SIZE_SHIFT,_kvm_pmm_counter);
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
