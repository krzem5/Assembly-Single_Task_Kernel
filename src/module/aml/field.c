#include <aml/object.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/pci/pci.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "aml_field"



aml_object_t* aml_field_read(aml_object_t* object){
	if (object->type!=AML_OBJECT_TYPE_FIELD_UNIT){
		return NULL;
	}
	// ERROR("R: flags=%X, address=%p, offset=%u, size=%u",object->field_unit.flags,object->field_unit.address,object->field_unit.offset,object->field_unit.size);
	if (object->field_unit.offset&7){
		panic("aml_field_read: sub-byte offsets not supported");
	}
	switch (object->field_unit.type){
		case 0x00:
			switch (object->field_unit.size){
				case 8:
					return aml_object_alloc_integer(*(const u8*)(object->field_unit.address+(object->field_unit.offset>>3)));
				case 16:
					return aml_object_alloc_integer(*(const u16*)(object->field_unit.address+(object->field_unit.offset>>3)));
				case 32:
					return aml_object_alloc_integer(*(const u32*)(object->field_unit.address+(object->field_unit.offset>>3)));
			}
			panic("aml_field_read: SystemMemory");
			break;
		case 0x01:
			switch (object->field_unit.size){
				case 1:
					return aml_object_alloc_integer(io_port_in8(object->field_unit.address+(object->field_unit.offset>>3))&1);
				case 8:
					return aml_object_alloc_integer(io_port_in8(object->field_unit.address+(object->field_unit.offset>>3)));
				case 16:
					return aml_object_alloc_integer(io_port_in16(object->field_unit.address+(object->field_unit.offset>>3)));
				case 32:
					return aml_object_alloc_integer(io_port_in32(object->field_unit.address+(object->field_unit.offset>>3)));
			}
			panic("aml_field_read: SystemIO");
			break;
		case 0x02:
			switch (object->field_unit.size){
				case 8:
					return aml_object_alloc_integer(pci_device_read_config8(object->field_unit.address+(object->field_unit.offset>>3)));
				case 16:
					return aml_object_alloc_integer(pci_device_read_config16(object->field_unit.address+(object->field_unit.offset>>3)));
				case 32:
					return aml_object_alloc_integer(pci_device_read_config32(object->field_unit.address+(object->field_unit.offset>>3)));
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
	if (object->type!=AML_OBJECT_TYPE_FIELD_UNIT){
		return 0;
	}
	// ERROR("W: flags=%X, address=%p, offset=%u, size=%u",object->field_unit.flags,object->field_unit.address,object->field_unit.offset,object->field_unit.size);
	if (object->field_unit.offset&7){
		panic("aml_field_write: sub-byte offsets not supported");
	}
	switch (object->field_unit.type){
		case 0x00:
			panic("aml_field_write: SystemMemory");
			break;
		case 0x01:
			if (value->type!=AML_OBJECT_TYPE_INTEGER){
				panic("aml_field_write: SystemIO: value is not an integer");
			}
			switch (object->field_unit.size){
				case 8:
					io_port_out8(object->field_unit.address+(object->field_unit.offset>>3),value->integer);
					return 1;
				case 16:
					io_port_out16(object->field_unit.address+(object->field_unit.offset>>3),value->integer);
					return 1;
				case 32:
					io_port_out32(object->field_unit.address+(object->field_unit.offset>>3),value->integer);
					return 1;
			}
			panic("aml_field_write: SystemIO");
			break;
		case 0x02:
			if (value->type!=AML_OBJECT_TYPE_INTEGER){
				panic("aml_field_write: SystemIO: value is not an integer");
			}
			switch (object->field_unit.size){
				case 8:
					pci_device_write_config8(object->field_unit.address+(object->field_unit.offset>>3),value->integer);
					return 1;
				case 16:
					pci_device_write_config16(object->field_unit.address+(object->field_unit.offset>>3),value->integer);
					return 1;
				case 32:
					pci_device_write_config32(object->field_unit.address+(object->field_unit.offset>>3),value->integer);
					return 1;
			}
			panic("aml_field_write: PCI_Config");
			break;
		case 0x03:
			panic("aml_field_write: EmbeddedControl");
			break;
		case 0x04:
			panic("aml_field_write: SMBus");
			break;
		case 0x05:
			panic("aml_field_write: System CMOS");
			break;
		case 0x06:
			panic("aml_field_write: PciBarTarget");
			break;
		case 0x07:
			panic("aml_field_write: IPMI");
			break;
		case 0x08:
			panic("aml_field_write: GeneralPurposeIO");
			break;
		case 0x09:
			panic("aml_field_write: GenericSerialBus");
			break;
		case 0x0A:
			panic("aml_field_write: PCC");
			break;
	}
	return NULL;
}
