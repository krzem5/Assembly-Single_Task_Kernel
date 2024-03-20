#include <kernel/acpi/structures.h>
#include <kernel/acpi/tpm2.h>
#include <kernel/aml/bus.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/tpm/tpm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "tpm"



#define TPM_COMMAND_BUFFER_SIZE PAGE_SIZE



#define	TPM_REG_ACCESS(locality) (0x0000|((locality)<<10))
#define	TPM_REG_INT_ENABLE(locality) (0x0002|((locality)<<10))
#define	TPM_REG_INTF_CAPS(locality) (0x0005|((locality)<<10))
#define	TPM_REG_STS(locality) (0x0006|((locality)<<10))
#define	TPM_REG_DATA_FIFO(locality) (0x009|((locality)<<10))
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



#define TPM2_ST_NO_SESSIONS 0x8001

#define TPM2_CC_SELF_TEST 0x0143
#define TPM2_CC_GET_CAPABILITY 0x017a

#define TPM2_CAP_COMMANDS 2
#define TPM2_CAP_PCRS 5
#define TPM2_CAP_TPM_PROPERTIES 6

#define TPM_PT_TOTAL_COMMANDS 0x0129

#define TPM2_RC_SUCCESS 0x0000
#define TPM2_RC_INITIALIZE 0x0100
#define TPM2_RC_TESTING 0x090a

#define TPM2_CC_FIRST 0x011f

#define TPM_ALG_SHA1 0x0004
#define TPM_ALG_SHA256 0x000b
#define TPM_ALG_SHA384 0x000c
#define TPM_ALG_SHA512 0x000d



typedef struct KERNEL_PACKED _TPM_COMMAND_HEADER{
	u16 tag;
	u32 length;
	union{
		u32 command_code;
		u32 return_code;
	};
	union{
		struct KERNEL_PACKED{
			u8 full_test;
		} self_test;
		struct KERNEL_PACKED{
			u32 capability;
			u32 property;
			u32 property_count;
		} get_capability;
		struct KERNEL_PACKED{
			u8 more_data;
			u32 subcap_id;
			u32 property_count;
			u32 property_id;
			u32 value;
		} get_capability_resp_tpm_properties;
	};
} tpm_command_header_t;



typedef struct KERNEL_PACKED _TPM_COMMAND2_HEADER{
	u16 tag;
	u32 length;
	union{
		u32 command_code;
		u32 return_code;
	};
} tpm_command_header2_t;



static omm_allocator_t* _tpm_allocator=NULL;
static pmm_counter_descriptor_t* _tpm_command_buffer_pmm_counter=NULL;



static void _chip_start(tpm_t* tpm){
	if ((tpm->regs[TPM_REG_ACCESS(0)]&(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY|TPM_ACCESS_REQUEST_USE))!=(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY)){
		tpm->regs[TPM_REG_ACCESS(0)]=TPM_ACCESS_REQUEST_USE;
		SPINLOOP((tpm->regs[TPM_REG_ACCESS(0)]&(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY|TPM_ACCESS_REQUEST_USE))!=(TPM_ACCESS_VALID|TPM_ACCESS_ACTIVE_LOCALITY));
	}
	tpm->regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
}



static void _chip_stop(tpm_t* tpm){
	tpm->regs[TPM_REG_ACCESS(0)]=TPM_ACCESS_ACTIVE_LOCALITY;
}



static void _send_data(tpm_t* tpm,const void* buffer,u32 buffer_size){
	if (!(tpm->regs[TPM_REG_STS(0)]&TPM_STS_COMMAND_READY)){
		tpm->regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
		SPINLOOP((tpm->regs[TPM_REG_STS(0)]&TPM_STS_COMMAND_READY)!=TPM_STS_COMMAND_READY);
	}
	u32 count=0;
	while (count<buffer_size-1){
		u32 burst_count;
		do{
			burst_count=(tpm->regs[TPM_REG_STS(0)]>>8)&0xffff;
		} while (!burst_count);
		if (buffer_size-count-1<burst_count){
			burst_count=buffer_size-count-1;
		}
		burst_count+=count;
		for (volatile u8* fifo=(volatile u8*)(tpm->regs+TPM_REG_DATA_FIFO(0));count<burst_count;count++){
			*fifo=*((const u8*)(buffer+count));
		}
		SPINLOOP((tpm->regs[TPM_REG_STS(0)]&TPM_STS_VALID)!=TPM_STS_VALID);
		if (!(tpm->regs[TPM_REG_STS(0)]&TPM_STS_DATA_EXPECT)){
			panic("End of data");
		}
	}
	*((u8*)(tpm->regs+TPM_REG_DATA_FIFO(0)))=*((const u8*)(buffer+count));
	SPINLOOP((tpm->regs[TPM_REG_STS(0)]&TPM_STS_VALID)!=TPM_STS_VALID);
	if (tpm->regs[TPM_REG_STS(0)]&TPM_STS_DATA_EXPECT){
		panic("Not enough data");
	}
}



static void _recv_data(tpm_t* tpm,void* buffer,u32 buffer_size){
	u32 count=0;
	while (count<buffer_size){
		SPINLOOP((tpm->regs[TPM_REG_STS(0)]&(TPM_STS_VALID|TPM_STS_DATA_AVAIL))!=(TPM_STS_VALID|TPM_STS_DATA_AVAIL));
		u32 burst_count;
		do{
			burst_count=(tpm->regs[TPM_REG_STS(0)]>>8)&0xffff;
		} while (!burst_count);
		if (buffer_size-count<burst_count){
			burst_count=buffer_size-count;
		}
		burst_count+=count;
		for (volatile u8* fifo=(volatile u8*)(tpm->regs+TPM_REG_DATA_FIFO(0));count<burst_count;count++){
			*((u8*)(buffer+count))=*fifo;
		}
	}
}



static void _transmit_message(tpm_t* tpm){
	volatile tpm_command_header_t* header=tpm->command_buffer;
	_send_data(tpm,tpm->command_buffer,__builtin_bswap32(header->length));
	tpm->regs[TPM_REG_STS(0)]=TPM_STS_GO;
	SPINLOOP((tpm->regs[TPM_REG_STS(0)]&(TPM_STS_VALID|TPM_STS_DATA_AVAIL))!=(TPM_STS_VALID|TPM_STS_DATA_AVAIL));
	_recv_data(tpm,tpm->command_buffer,sizeof(tpm_command_header2_t));
	if (__builtin_bswap32(header->length)>TPM_COMMAND_BUFFER_SIZE){
		panic("Response too large");
	}
	_recv_data(tpm,tpm->command_buffer+sizeof(tpm_command_header2_t),__builtin_bswap32(header->length)-sizeof(tpm_command_header2_t));
	tpm->regs[TPM_REG_STS(0)]=TPM_STS_COMMAND_READY;
}



static void _probe_tpm2(tpm_t* tpm){
	tpm_command_header_t* header=tpm->command_buffer;
	header->tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	header->length=__builtin_bswap32(sizeof(tpm_command_header2_t)+3*sizeof(u32));
	header->command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
	header->get_capability.capability=__builtin_bswap32(TPM2_CAP_TPM_PROPERTIES);
	header->get_capability.property=__builtin_bswap32(TPM_PT_TOTAL_COMMANDS);
	header->get_capability.property_count=__builtin_bswap32(1);
	_transmit_message(tpm);
	if (__builtin_bswap16(header->tag)==TPM2_ST_NO_SESSIONS){
		tpm->flags|=TPM_FLAG_VERSION_2;
	}
	INFO("Detected chip version: %s",((tpm->flags&TPM_FLAG_VERSION_2)?"2.0":"1.2"));
}



static _Bool _self_test_tpm2(tpm_t* tpm){
	_Bool full_self_test=0;
_retry_self_test:
	tpm_command_header_t* header=tpm->command_buffer;
	header->tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
	header->length=__builtin_bswap32(sizeof(tpm_command_header2_t)+sizeof(u8));
	header->command_code=__builtin_bswap32(TPM2_CC_SELF_TEST);
	header->self_test.full_test=full_self_test;
	_transmit_message(tpm);
	if (__builtin_bswap32(header->return_code)==TPM2_RC_SUCCESS||__builtin_bswap32(header->return_code)==TPM2_RC_INITIALIZE||__builtin_bswap32(header->return_code)==TPM2_RC_TESTING){
		return __builtin_bswap32(header->return_code)==TPM2_RC_INITIALIZE;
	}
	if (!full_self_test){
		full_self_test=1;
		goto _retry_self_test;
	}
	panic("TPM2 self-test failed");
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
	tpm_t* tpm=omm_alloc(_tpm_allocator);
	tpm->flags=0;
	tpm->regs=(void*)vmm_identity_map(res->memory_region.base,res->memory_region.size);
	tpm->command_buffer=(void*)(pmm_alloc(pmm_align_up_address(TPM_COMMAND_BUFFER_SIZE)>>PAGE_SIZE_SHIFT,_tpm_command_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	INFO("Memory range: %p - %p",res->memory_region.base,res->memory_region.base+res->memory_region.size);
	INFO("Vendor ID: %x, Device ID: %x, Revision ID: %x",tpm->regs[TPM_REG_DID_VID(0)]&0xffff,tpm->regs[TPM_REG_DID_VID(0)]>>16,tpm->regs[TPM_REG_RID(0)]);
	SPINLOOP(!(tpm->regs[TPM_REG_ACCESS(0)]&TPM_ACCESS_VALID));
	u32 interrupt_mask=tpm->regs[TPM_REG_INT_ENABLE(0)];
	u32 capabilites=tpm->regs[TPM_REG_INTF_CAPS(0)];
	INFO("Interrupt mask: %x, Capabilites: %x",interrupt_mask,capabilites);
	interrupt_mask=(interrupt_mask|TPM_INTF_CMD_READY_INT|TPM_INTF_LOCALITY_CHANGE_INT|TPM_INTF_STS_VALID_INT|TPM_INTF_DATA_AVAIL_INT)&(~TPM_GLOBAL_INT_ENABLE);
	_chip_start(tpm);
	tpm->regs[TPM_REG_INT_ENABLE(0)]=interrupt_mask;
	_probe_tpm2(tpm);
	if (tpm->flags&TPM_FLAG_VERSION_2){
		_Bool require_initialization=_self_test_tpm2(tpm);
		if (require_initialization){
			panic("TPM2 requires initialization");
		}
		tpm_command_header_t* header=tpm->command_buffer;
		header->tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
		header->length=__builtin_bswap32(sizeof(tpm_command_header2_t)+3*sizeof(u32));
		header->command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
		header->get_capability.capability=__builtin_bswap32(TPM2_CAP_TPM_PROPERTIES);
		header->get_capability.property=__builtin_bswap32(TPM_PT_TOTAL_COMMANDS);
		header->get_capability.property_count=__builtin_bswap32(1);
		_transmit_message(tpm);
		if (!__builtin_bswap32(header->get_capability_resp_tpm_properties.property_count)){
			panic("No properties returned");
		}
		u32 command_count=__builtin_bswap32(header->get_capability_resp_tpm_properties.value);
		header->tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
		header->length=__builtin_bswap32(sizeof(tpm_command_header2_t)+3*sizeof(u32));
		header->command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
		header->get_capability.capability=__builtin_bswap32(TPM2_CAP_COMMANDS);
		header->get_capability.property=__builtin_bswap32(TPM2_CC_FIRST);
		header->get_capability.property_count=__builtin_bswap32(command_count);
		_transmit_message(tpm);
		if (__builtin_bswap32(*((const u32*)(tpm->command_buffer+sizeof(tpm_command_header2_t)+5)))!=command_count){
			panic("Inconsistent command count");
		}
		for (u32 i=0;i<command_count;i++){
			u32 attrs=__builtin_bswap32(*((const u32*)(tpm->command_buffer+sizeof(tpm_command_header2_t)+i*4+9)));
			WARN("CC: %x, Attrs: %x",attrs&0xffff,attrs>>16);
		}
		header->tag=__builtin_bswap16(TPM2_ST_NO_SESSIONS);
		header->length=__builtin_bswap32(sizeof(tpm_command_header2_t)+3*sizeof(u32));
		header->command_code=__builtin_bswap32(TPM2_CC_GET_CAPABILITY);
		header->get_capability.capability=__builtin_bswap32(TPM2_CAP_PCRS);
		header->get_capability.property=__builtin_bswap32(0);
		header->get_capability.property_count=__builtin_bswap32(1);
		_transmit_message(tpm);
		u32 bank_count=__builtin_bswap32(*((const u32*)(tpm->command_buffer+sizeof(tpm_command_header2_t)+5)));
		const void* ptr=tpm->command_buffer+sizeof(tpm_command_header2_t)+9;
		for (u32 i=0;i<bank_count;i++){
			u16 hash_alg=__builtin_bswap16(*((const u16*)ptr));
			u8 size_of_selection=*((const u8*)(ptr+2));
			for (u32 j=0;j<size_of_selection;j++){
				if (!(*((const u8*)(ptr+j+3)))){
					continue;
				}
				u32 digest_size=0;
				if (hash_alg==TPM_ALG_SHA1){
					digest_size=5;
				}
				else if (hash_alg==TPM_ALG_SHA256){
					digest_size=8;
				}
				else if (hash_alg==TPM_ALG_SHA384){
					digest_size=12;
				}
				else if (hash_alg==TPM_ALG_SHA512){
					digest_size=16;
				}
				else{
					panic("Unknown hash algorithm digest size");
				}
				WARN("%u -> %u",hash_alg,digest_size);
				break;
			}
			ptr+=sizeof(u16)+sizeof(u8)+size_of_selection;
		}
	}
	else{
		panic("TPM 1.2 init");
	}
	_chip_stop(tpm);
	// panic("a");
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
	_tpm_allocator=omm_init("tpm",sizeof(tpm_t),8,1,pmm_alloc_counter("omm_tpm"));
	spinlock_init(&(_tpm_allocator->lock));
	_tpm_command_buffer_pmm_counter=pmm_alloc_counter("tpm_command_buffer");
	aml_bus_register_device_driver(&_tmp_aml_bus_device_driver);
}
