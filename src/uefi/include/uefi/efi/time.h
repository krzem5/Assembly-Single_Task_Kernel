#ifndef _UEFI_EFI_TIME_H_
#define _UEFI_EFI_TIME_H_ 1
#include <uefi/efi/types.h>



typedef struct _EFI_TIME{
	u16 Year;
	u8 Month;
	u8 Day;
	u8 Hour;
	u8 Minute;
	u8 Second;
	u8 Pad1;
	u32 Nanosecond;
	s16 TimeZone;
	u8 Daylight;
	u8 Pad2;
} efi_time_t;



#endif
