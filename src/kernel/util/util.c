#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



KERNEL_PUBLIC void KERNEL_NORETURN KERNEL_NOCOVERAGE panic(const char* error){
	log("\x1b[1m\x1b[1m\x1b[38;2;192;28;40mFatal error: %s\x1b[0m\n",error);
	io_port_out16(0x604,0x2000);
	for (;;);
}
