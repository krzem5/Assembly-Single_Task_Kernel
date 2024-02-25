#define DECLARE_SECTION_START(name) PROVIDE_HIDDEN(__KERNEL_SECTION_##name##_START__ = .)
#define DECLARE_SECTION_END(name) PROVIDE_HIDDEN(__KERNEL_SECTION_##name##_END__ = .)



ENTRY(_start)
SECTIONS{
	. = 0;
	DECLARE_SECTION_START(kernel);
	DECLARE_SECTION_START(kernel_ue);
	.kernel_ue : AT(ADDR(.kernel_ue)){
		*(.entry)
		*(.etext .etext*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_ue);
	DECLARE_SECTION_START(kernel_ur);
	.kernel_ur : AT(ADDR(.kernel_ur)){
		*(.erdata .erdata*)
		. = ALIGN(8);
		DECLARE_SECTION_START(cpu_local);
		*(.cpulocal)
		. = ALIGN(8);
		DECLARE_SECTION_END(cpu_local);
		DECLARE_SECTION_START(early_early_initializers);
		*(.eeinitializer)
		. = ALIGN(8);
		DECLARE_SECTION_END(early_early_initializers);
		DECLARE_SECTION_START(early_initializers);
		*(.einitializer)
		. = ALIGN(8);
		DECLARE_SECTION_END(early_initializers);
		DECLARE_SECTION_START(initializers);
		*(.initializer)
		. = ALIGN(8);
		DECLARE_SECTION_END(initializers);
		. = ALIGN(0x1000);
		*(.ap_startup)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_ur);
	DECLARE_SECTION_START(kernel_uw);
	.kernel_uw : AT(ADDR(.kernel_uw)){
		*(.edata .edata*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_uw);
	DECLARE_SECTION_START(kernel_ex);
	.kernel_ex : AT(ADDR(.kernel_ex)){
		*(.text .text*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_ex);
	DECLARE_SECTION_START(kernel_nx);
	.kernel_nx : AT(ADDR(.kernel_nx)){
		*(.rodata .rodata* .rdata .rdata*)
		. = ALIGN(8);
		DECLARE_SECTION_START(gcov_info);
#ifdef _KERNEL_COVERAGE_BUILD_
		*(.gcov_info)
#endif
		. = ALIGN(8);
		DECLARE_SECTION_END(gcov_info);
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_nx);
	DECLARE_SECTION_START(kernel_rw);
	.kernel_rw : AT(ADDR(.kernel_rw)){
		*(.data .data*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_rw);
	DECLARE_SECTION_START(kernel_iw);
	.kernel_iw : AT(ADDR(.kernel_iw)){
		*(.idata .idata*)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_iw);
	DECLARE_SECTION_START(kernel_zw);
	.kernel_zw : AT(ADDR(.kernel_zw)){
		*(.bss)
		. = ALIGN(0x1000);
	}
	DECLARE_SECTION_END(kernel_zw);
	DECLARE_SECTION_END(kernel);
#ifndef _KERNEL_DEBUG_BUILD_
	/DISCARD/ : {
		*(*)
	}
#endif
}
