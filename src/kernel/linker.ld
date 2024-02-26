ENTRY(_start)
SECTIONS{
	. = 0;
	.kernel_ue : {
		*(.entry)
		*(.etext .etext*)
	}
	.kernel_ur : {
		*(.ap_startup)
		. = ALIGN(8);
		*(.erdata .erdata*)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__KERNEL_SECTION_cpu_local_START__ = .);
		*(.cpulocal)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__KERNEL_SECTION_cpu_local_END__ = .);
		PROVIDE_HIDDEN(__KERNEL_SECTION_early_early_initializers_START__ = .);
		*(.eeinitializer)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__KERNEL_SECTION_early_early_initializers_END__ = .);
		PROVIDE_HIDDEN(__KERNEL_SECTION_early_initializers_START__ = .);
		*(.einitializer)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__KERNEL_SECTION_early_initializers_END__ = .);
		PROVIDE_HIDDEN(__KERNEL_SECTION_initializers_START__ = .);
		*(.initializer)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__KERNEL_SECTION_initializers_END__ = .);
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
		PROVIDE_HIDDEN(__KERNEL_SECTION_gcov_info_START__ = .);
		*(.gcov_info)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__KERNEL_SECTION_gcov_info_END__ = .);
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
