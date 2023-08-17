#include <kernel/aml/runtime.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aml_bus"



static void _parse_crs(const u8* data,u32 length){
	for (u32 i=0;i<length;){
		u8 type=data[i];
		u8 code;
		u16 length;
		if (type>>7){
			code=(type<<1)|1;
			length=data[i+1]|(data[i+2]<<1);
			i+=3;
		}
		else{
			code=(type&0x78)>>2;
			length=type&7;
			i++;
		}
		switch (code){
			case 0x08:
				WARN("Unimplemented _CRS tag: IRQ Format Descriptor");
				break;
			case 0x0a:
				WARN("Unimplemented _CRS tag: DMA Format Descriptor");
				break;
			case 0x0c:
				WARN("Unimplemented _CRS tag: Start Dependent Functions Descriptor");
				break;
			case 0x0e:
				WARN("Unimplemented _CRS tag: End Dependent Functions Descriptor");
				break;
			case 0x10:
				WARN("Unimplemented _CRS tag: I/O Port Descriptor");
				break;
			case 0x12:
				WARN("Unimplemented _CRS tag: Fixed Location I/O Port Descriptor");
				break;
			case 0x14:
				WARN("Unimplemented _CRS tag: Fixed DMA Descriptor");
				break;
			case 0x1c:
				WARN("Unimplemented _CRS tag: Vendor Defined Descriptor");
				break;
			case 0x1e:
				return;
			case 0x03:
				WARN("Unimplemented _CRS tag: 24-Bit Memory Range Descriptor");
				break;
			case 0x05:
				WARN("Unimplemented _CRS tag: Generic Register Descriptor");
				break;
			case 0x09:
				WARN("Unimplemented _CRS tag: Vendor-Defined Descriptor");
				break;
			case 0x0b:
				WARN("Unimplemented _CRS tag: 32-Bit Memory Range Descriptor");
				break;
			case 0x0d:
				WARN("Unimplemented _CRS tag: 32-Bit Fixed Memory Range Descriptor");
				break;
			case 0x0f:
				WARN("Unimplemented _CRS tag: Address Space Resource Descriptors");
				break;
			case 0x11:
				WARN("Unimplemented _CRS tag: Word Address Space Descriptor");
				break;
			case 0x13:
				WARN("Unimplemented _CRS tag: Extended Interrupt Descriptor");
				break;
			case 0x15:
				WARN("Unimplemented _CRS tag: QWord Address Space Descriptor");
				break;
			case 0x17:
				WARN("Unimplemented _CRS tag: Extended Address Space Descriptor");
				break;
			case 0x19:
				WARN("Unimplemented _CRS tag: GPIO Connection Descriptor");
				break;
			case 0x1b:
				WARN("Unimplemented _CRS tag: Pin Function Descriptor");
				break;
			case 0x1d:
				WARN("Unimplemented _CRS tag: GenericSerialBus Connection Descriptors");
				break;
			case 0x1f:
				WARN("Unimplemented _CRS tag: Pin Configuration Descriptor");
				break;
			case 0x21:
				WARN("Unimplemented _CRS tag: Pin Group Descriptor");
				break;
			case 0x23:
				WARN("Unimplemented _CRS tag: Pin Group Function Descriptor");
				break;
			case 0x25:
				WARN("Unimplemented _CRS tag: Pin Group Configuration Descriptor");
				break;
			default:
				WARN("Unknon _CRS tag: %x",code);
				break;
		}
		i+=length;
	}
}



static void _parse_device(aml_node_t* device){
	LOG("%s",device);
	aml_node_t* hid=aml_runtime_evaluate_node(aml_runtime_get_node(device,"_HID"));
	if (hid){
		aml_runtime_print_node(hid);
	}
	aml_node_t* adr=aml_runtime_evaluate_node(aml_runtime_get_node(device,"_ADR"));
	if (adr){
		aml_runtime_print_node(adr);
	}
	aml_node_t* crs=aml_runtime_evaluate_node(aml_runtime_get_node(device,"_CRS"));
	if (crs&&crs->type==AML_NODE_TYPE_BUFFER){
		_parse_crs(crs->data.buffer.data,crs->data.buffer.length);
	}
}



static void _enumerate_bus(aml_node_t* bus){
	for (aml_node_t* device=bus->child;device;device=device->next){
		if (device->type!=AML_NODE_TYPE_DEVICE){
			continue;
		}
		u64 status=0xf;
		aml_node_t* status_node=aml_runtime_evaluate_node(aml_runtime_get_node(device,"_STA"));
		if (status_node){
			status=status_node->data.integer;
		}
		if (!(status&8)){
			continue;;
		}
		if (status&1){
			_parse_device(device);
		}
		_enumerate_bus(device);
	}
}



void aml_bus_enumerate(void){
	LOG("Enumerating AML bus devices...");
	_enumerate_bus(aml_runtime_get_node(aml_root_node,"\\_SB_"));
	for (;;);
}
