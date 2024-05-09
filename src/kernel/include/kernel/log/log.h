#ifndef _KERNEL_LOG_LOG_H_
#define _KERNEL_LOG_LOG_H_ 1



#define INFO(template,...) _LOG("\x1b[38;2;182;182;182m",template,##__VA_ARGS__)
#define LOG(template,...) _LOG("\x1b[1m\x1b[38;2;242;242;242m",template,##__VA_ARGS__)
#define WARN(template,...) _LOG("\x1b[38;2;231;211;72m",template,##__VA_ARGS__)
#define ERROR(template,...) _LOG("\x1b[1m\x1b[38;2;231;72;86m",template,##__VA_ARGS__)

#define _LOG(color,template,...) log("\x1b[38;2;65;65;65m["KERNEL_LOG_NAME"] "color template"\x1b[0m\n",##__VA_ARGS__)



void _log_untraced(const char* template,...);



void log(const char* template,...);



#endif
