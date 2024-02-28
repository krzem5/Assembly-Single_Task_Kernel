#include <kernel/kvm/kvm.h>
#include <kernel/log/log.h>
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
	WARN("features = %p",features);
	WARN("feature_hints = %p",feature_hints);
}
