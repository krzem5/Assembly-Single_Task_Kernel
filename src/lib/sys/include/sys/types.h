#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_ 1



#define SYS_PUBLIC __attribute__((visibility("default")))
#define SYS_PACKED __attribute__((packed))
#define SYS_CONSTRUCTOR __attribute__((constructor))
#define SYS_DESTRUCTOR __attribute__((destructor))
#define SYS_NOCOVERAGE __attribute__((no_instrument_function,no_profile_instrument_function))
#define SYS_NORETURN __attribute__((noreturn))



typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned int u32;
typedef unsigned long long int u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed int s32;
typedef signed long long int s64;



#endif
