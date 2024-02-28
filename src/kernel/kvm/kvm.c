#include <kernel/kvm/kvm.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/msr/msr.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "kvm"



KERNEL_EARLY_INIT(){
	u32 features;
	u32 feature_hints;
	if (!_kvm_get_flags(&features,&feature_hints)){
		return;
	}
	LOG("KVM hypervisor detected");
	WARN("kvm.features = %p",features);
	WARN("kvm.feature_hints = %p",feature_hints);
	pmm_counter_descriptor_t* pmm_counter=pmm_alloc_counter("kvm");
	u64 physical_time_info=pmm_alloc(pmm_align_up_address(sizeof(kvm_time_info_t))>>PAGE_SIZE_SHIFT,pmm_counter,0);
	msr_write(KVM_MSR_SYSTEM_TIME_NEW,physical_time_info|KVM_MSR_FLAG_ENABLED);
	kvm_time_info_t* info=(void*)(physical_time_info+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	u64 value=(1000000000ull<<32)/info->tsc_to_system_mul;
	WARN("TSC frequency: %u",(info->tsc_shift<0?value<<(-info->tsc_shift):value>>info->tsc_shift));
	msr_write(KVM_MSR_SYSTEM_TIME_NEW,0);
	pmm_dealloc(physical_time_info,pmm_align_up_address(sizeof(kvm_time_info_t))>>PAGE_SIZE_SHIFT,pmm_counter);
	// panic("KVM");
}
