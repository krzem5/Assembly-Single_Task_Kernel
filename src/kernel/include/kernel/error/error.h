#ifndef _KERNEL_ERROR_ERROR_H_
#define _KERNEL_ERROR_ERROR_H_ 1
#include <kernel/types.h>



#define IS_ERROR(x) ((x)<0)

#define ERROR_OK 0x0000
#define ERROR_INVALID_SYSCALL (-0x0001)
#define ERROR_INVALID_ARGUMENT(index) (-(0x0002|((index)<<16)))
#define ERROR_NOT_FOUND (-0x0003)
#define ERROR_INVALID_HANDLE (-0x0004)
#define ERROR_NO_ACL (-0x0005)
#define ERROR_DENIED (-0x0006)
#define ERROR_UNSUPPORTED_OPERATION (-0x0007)
#define ERROR_NO_SPACE (-0x0008)
#define ERROR_EOF (-0x0009)



#endif
