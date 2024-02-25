#define DECLARE_SECTION_START(name) PROVIDE_HIDDEN(__KERNEL_SECTION_##name##_START__ = .)
#define DECLARE_SECTION_END(name) PROVIDE_HIDDEN(__KERNEL_SECTION_##name##_END__ = .)



ENTRY(_start)
SECTIONS{
	. = 0;
	.kernel_ue : {
		*(.entry)
		*(.etext .etext*)
	}
	.kernel_ur : {
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
	}
	.kernel_uw : {
		*(.edata .edata*)
	}
	.kernel_ex : {
		*(.text .text*)
	}
	.kernel_nx : {
		*(.rodata .rodata* .rdata .rdata*)
		. = ALIGN(8);
		DECLARE_SECTION_START(gcov_info);
		*(.gcov_info)
		. = ALIGN(8);
		DECLARE_SECTION_END(gcov_info);
	}
	.kernel_rw : {
		*(.data .data* .bss .bss*)
	}
	.kernel_iw : {
		*(.idata .idata*)
	}
	/DISCARD/ : {
		*(.comment .gnu .gnu.* .note .note.*)
	}
}
