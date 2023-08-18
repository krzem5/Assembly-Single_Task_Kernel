#if KERNEL_COVERAGE_ENABLED
#include <kernel/acpi/fadt.h>
#include <kernel/io/io.h>
#include <kernel/log/log.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "coverage"




extern u64 __KERNEL_GCOV_INFO_START__[];
extern u64 __KERNEL_GCOV_INFO_END__[];



void KERNEL_NORETURN syscall_dump_coverage_data(syscall_registers_t* regs){
	LOG("Dumping coverage information...");
	INFO("Initializing serial port...");
	io_port_out8(0x2f9,0x00);
	io_port_out8(0x2fb,0x80);
	io_port_out8(0x2f8,0x03);
	io_port_out8(0x2f9,0x00);
	io_port_out8(0x2fb,0x03);
	io_port_out8(0x2fa,0xc7);
	io_port_out8(0x2fc,0x03);
	LOG("%p %p",__KERNEL_GCOV_INFO_START__,__KERNEL_GCOV_INFO_END__);
	for (struct gcov_info* gi_ptr=(struct gcov_info*)(u64)__KERNEL_GCOV_INFO_START__;gi_ptr<(struct gcov_info*)(u64)__KERNEL_GCOV_INFO_END__;gi_ptr++){
		LOG("%p",gi_ptr);
	}
	acpi_fadt_shutdown(0);
}



#endif
