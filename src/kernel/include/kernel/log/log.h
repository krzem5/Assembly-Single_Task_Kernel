#ifndef _KERNEL_LOG_LOG_H_
#define _KERNEL_LOG_LOG_H_ 1



#define INFO(template,...) _LOG("\x1b[37m",template,##__VA_ARGS__)
#define LOG(template,...) _LOG("\x1b[1;97m",template,##__VA_ARGS__)
#define WARN(template,...) _LOG("\x1b[93m",template,##__VA_ARGS__)
#define ERROR(template,...) _LOG("\x1b[1;91m",template,##__VA_ARGS__)

#define _LOG(color,template,...) log("\x1b[90m["KERNEL_LOG_NAME"] "color template"\x1b[0m\n",##__VA_ARGS__)



void log(const char* template,...);



#endif
