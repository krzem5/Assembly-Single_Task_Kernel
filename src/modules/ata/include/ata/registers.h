#ifndef _ATA_REGISTERS_H_
#define _ATA_REGISTERS_H_ 1



// ATA registers
#define ATA_REG_DATA 0
#define ATA_REG_ERROR 1
#define ATA_REG_NSECTOR 2
#define ATA_REG_LBA0 3
#define ATA_REG_LBA1 4
#define ATA_REG_LBA2 5
#define ATA_REG_DRV_HEAD 6
#define ATA_REG_STATUS 7
#define ATA_REG_COMMAND 7
#define ATA_REG_DEV_CTL 518

// ATA commands
#define ATA_CMD_PACKET 0xa0
#define ATA_CMD_ATAPI_IDENTIFY 0xa1
#define ATA_CMD_IDENTIFY 0xec

// ATAPI commands
#define ATAPI_CMD_READ_CAPACITY 0x25
#define ATAPI_CMD_READ_SECTORS 0xa8

// Status flags
#define STATUS_ERR 0x01
#define STATUS_DRQ 0x08
#define STATUS_BSY 0x80

// DEV_CTL flags
#define DEV_CTL_SRST 0x04



#endif
