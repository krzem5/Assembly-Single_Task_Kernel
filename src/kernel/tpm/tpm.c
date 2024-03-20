#include <kernel/acpi/structures.h>
#include <kernel/acpi/tpm2.h>
#include <kernel/aml/bus.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "tpm"



static _Bool _init_aml_device(aml_bus_device_t* device){
	if (!acpi_tpm2||acpi_tpm2->start_method!=ACPI_TPM2_START_METHOD_MEMORY_MAPPED){
		return 0;
	}
	const aml_bus_device_resource_t* res=aml_bus_device_get_resource(device,AML_BUS_RESOURCE_TYPE_MEMORY_REGION,0);
	if (!res){
		return 0;
	}
	ERROR("TPM2: %p - %p",res->memory_region.base,res->memory_region.base+res->memory_region.size);
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
