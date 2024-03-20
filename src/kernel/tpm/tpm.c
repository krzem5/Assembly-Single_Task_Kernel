#include <kernel/acpi/structures.h>
#include <kernel/acpi/tpm2.h>
#include <kernel/aml/bus.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "tpm"



#define	TPM_REG_ACCESS(locality) (0x0000|((locality)<<10))
#define	TPM_REG_INT_ENABLE(locality) (0x0002|((locality)<<10))
#define	TPM_REG_INTF_CAPS(locality) (0x0005|((locality)<<10))
#define	TPM_REG_STS(locality) (0x0006|((locality)<<10))
#define	TPM_REG_DID_VID(locality) (0x03c0|((locality)<<10))
#define	TPM_REG_RID(locality) (0x03c1|((locality)<<10))

#define TPM_ACCESS_REQUEST_USE 0x02
#define TPM_ACCESS_REQUEST_PENDING 0x04
#define TPM_ACCESS_ACTIVE_LOCALITY 0x20
#define TPM_ACCESS_VALID 0x80

#define TPM_INTF_DATA_AVAIL_INT 0x001
#define TPM_INTF_STS_VALID_INT 0x002
#define TPM_INTF_LOCALITY_CHANGE_INT 0x004
#define TPM_INTF_INT_LEVEL_HIGH 0x008
#define TPM_INTF_INT_LEVEL_LOW 0x010
#define TPM_INTF_INT_EDGE_RISING 0x020
#define TPM_INTF_INT_EDGE_FALLING 0x040
#define TPM_INTF_CMD_READY_INT 0x080
#define TPM_INTF_BURST_COUNT_STATIC 0x100
#define TPM_GLOBAL_INT_ENABLE 0x80000000

#define TPM_STS_RESPONSE_RETRY 0x02
#define TPM_STS_DATA_EXPECT 0x08
#define TPM_STS_DATA_AVAIL 0x10
#define TPM_STS_GO 0x20
#define TPM_STS_COMMAND_READY 0x40
#define TPM_STS_VALID 0x80

#define TPM_STS_READ_ZERO 0x23



static void _chip_start(volatile u32* regs){
	if ((regs[TPM_REG_ACCESS(0)]&(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY|TPM_ACCESS_REQUEST_USE))==(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY)){
		regs[TPM_REG_ACCESS(0)]=TPM_ACCESS_REQUEST_USE;
		SPINLOOP((regs[TPM_REG_ACCESS(0)]&(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY|TPM_ACCESS_REQUEST_USE))==(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY));
	}
	regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
}



static void _chip_stop(volatile u32* regs){
	regs[TPM_REG_ACCESS(0)]=TPM_ACCESS_ACTIVE_LOCALITY;
}



static _Bool _init_aml_device(aml_bus_device_t* device){
	if (!acpi_tpm2||acpi_tpm2->start_method!=ACPI_TPM2_START_METHOD_MEMORY_MAPPED){
		return 0;
	}
	const aml_bus_device_resource_t* res=aml_bus_device_get_resource(device,AML_BUS_RESOURCE_TYPE_MEMORY_REGION,0);
	if (!res){
		return 0;
	}
	LOG("Found TMP device: %s (SystemBus)",device->device->name);
	volatile u32* regs=(void*)vmm_identity_map(res->memory_region.base,res->memory_region.size);
	INFO("Memory range: %p - %p",res->memory_region.base,res->memory_region.base+res->memory_region.size);
	INFO("Vendor ID: %x, Device ID: %x, Revision ID: %x",regs[TPM_REG_DID_VID(0)]&0xffff,regs[TPM_REG_DID_VID(0)]>>16,regs[TPM_REG_RID(0)]);
	SPINLOOP(!(regs[TPM_REG_ACCESS(0)]&TPM_ACCESS_VALID));
	u32 interrupt_mask=regs[TPM_REG_INT_ENABLE(0)];
	u32 capabilites=regs[TPM_REG_INTF_CAPS(0)];
	INFO("Interrupt mask: %x, Capabilites: %x",interrupt_mask,capabilites);
	interrupt_mask=(interrupt_mask|TPM_INTF_CMD_READY_INT|TPM_INTF_LOCALITY_CHANGE_INT|TPM_INTF_STS_VALID_INT|TPM_INTF_DATA_AVAIL_INT)&(~TPM_GLOBAL_INT_ENABLE);
	_chip_start(regs);
	regs[TPM_REG_INT_ENABLE(0)]=interrupt_mask;
	_chip_stop(regs);
	return 1;
}



static void _deinit_aml_device(aml_bus_device_t* device){
	return;
}



static const aml_bus_device_driver_t _tmp_aml_bus_device_driver={
	"TPM2",
	AML_BUS_ADDRESS_TYPE_HID_STR,
	{.hid_str="MSFT0101"},
	_init_aml_device,
	_deinit_aml_device
};



KERNEL_INIT(){
	LOG("Initializing TPM driver...");
	aml_bus_register_device_driver(&_tmp_aml_bus_device_driver);
}
