#include <aml/object.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aml_field"



aml_object_t* aml_field_read(aml_object_t* object){
	if (object->type!=AML_OBJECT_TYPE_FIELD_UNIT){
		return NULL;
	}
	ERROR("flags=%X, address=%p, offset=%u, size=%u",object->field_unit.flags,object->field_unit.address,object->field_unit.offset,object->field_unit.size);
	switch (object->field_unit.type){
		case 0x00:
			panic("aml_field_read: SystemMemory");
			break;
		case 0x01:
			panic("aml_field_read: SystemIO");
			break;
		case 0x02:
			if (object->field_unit.offset&7){
				panic("aml_field_read: PCI_Config: sub-byte offsets not supported");
			}
			switch (object->field_unit.size){
				case 8:
					return aml_object_alloc_integer(pci_device_read_config(object->field_unit.address+(object->field_unit.offset>>3))&255);
			}
			panic("aml_field_read: PCI_Config");
			break;
		case 0x03:
			panic("aml_field_read: EmbeddedControl");
			break;
		case 0x04:
			panic("aml_field_read: SMBus");
			break;
		case 0x05:
			panic("aml_field_read: System CMOS");
			break;
		case 0x06:
			panic("aml_field_read: PciBarTarget");
			break;
		case 0x07:
			panic("aml_field_read: IPMI");
			break;
		case 0x08:
			panic("aml_field_read: GeneralPurposeIO");
			break;
		case 0x09:
			panic("aml_field_read: GenericSerialBus");
			break;
		case 0x0A:
			panic("aml_field_read: PCC");
			break;
	}
	return NULL;
}



_Bool aml_field_write(aml_object_t* object,aml_object_t* value){
	return 0;
}
