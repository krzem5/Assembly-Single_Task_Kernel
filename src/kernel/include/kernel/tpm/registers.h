#ifndef _KERNEL_TPM_REGISTERS_H_
#define _KERNEL_TPM_REGISTERS_H_ 1



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



#endif
