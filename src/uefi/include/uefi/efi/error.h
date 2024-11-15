#ifndef _UEFI_EFI_ERROR_H_
#define _UEFI_EFI_ERROR_H_ 1
#include <common/types.h>



#define _EFI_BUILD_ERROR(a) (a|0x8000000000000000ull)

#define EFI_IS_ERROR(a) (((s64)a)<0)

#define EFI_SUCCESS 0
#define EFI_INVALID_PARAMETER _EFI_BUILD_ERROR(2)
#define EFI_BUFFER_TOO_SMALL _EFI_BUILD_ERROR(5)



#endif
