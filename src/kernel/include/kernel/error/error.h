#ifndef _KERNEL_ERROR_ERROR_H_
#define _KERNEL_ERROR_ERROR_H_ 1
#include <kernel/types.h>



#define IS_ERROR(x) ((x)<0)

#define _ERROR(x) (-((s64)(x)))

#define ERROR_OK _ERROR(0x0000)
#define ERROR_INVALID_SYSCALL _ERROR(0x0001)
#define ERROR_INVALID_ARGUMENT(index) _ERROR((0x0002|((index)<<16)))
#define ERROR_NOT_FOUND _ERROR(0x0003)
#define ERROR_INVALID_HANDLE _ERROR(0x0004)
#define ERROR_NO_ACL _ERROR(0x0005)
#define ERROR_DENIED _ERROR(0x0006)
#define ERROR_UNSUPPORTED_OPERATION _ERROR(0x0007)
#define ERROR_NO_SPACE _ERROR(0x0008)
#define ERROR_EOF _ERROR(0x0009)



typedef u64 error_t;



#endif
