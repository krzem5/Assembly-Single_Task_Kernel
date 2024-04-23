#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_ 1
#include <common/types.h>



#define SYS_PUBLIC __attribute__((visibility("default")))
#define SYS_PACKED __attribute__((packed))
#define SYS_CONSTRUCTOR __attribute__((constructor))
#define SYS_DESTRUCTOR __attribute__((destructor))
#define SYS_NOCOVERAGE __attribute__((no_instrument_function,no_profile_instrument_function))
#define SYS_NORETURN __attribute__((noreturn))
#define __SYS_SIGNATURE static const u8 __attribute__((used,section(".signature"))) _user_signature[4096]



#endif
