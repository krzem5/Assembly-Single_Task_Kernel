#ifndef _KERNEL_LOG_LOG_H_
#define _KERNEL_LOG_LOG_H_ 1
#include <kernel/print/print.h>
#include <kernel/types.h>



#define INFO(template,...) _LOG("\x1b[38;2;182;182;182m",template,##__VA_ARGS__)
#define LOG(template,...) _LOG("\x1b[1m\x1b[38;2;242;242;242m",template,##__VA_ARGS__)
#define WARN(template,...) _LOG("\x1b[38;2;231;211;72m",template,##__VA_ARGS__)
#define ERROR(template,...) _LOG("\x1b[1m\x1b[38;2;231;72;86m",template,##__VA_ARGS__)

#define _LOG(color,template,...) print("\x1b[38;2;65;65;65m["__SHORT_FILE_NAME__"] "color template"\x1b[0m\n",##__VA_ARGS__)



#endif
