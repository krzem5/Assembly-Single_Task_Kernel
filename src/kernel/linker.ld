ENTRY(_start)
SECTIONS{
	. = 0;
	.kernel_ue : {
		*(.entry)
		*(.etext .etext*)
		. = ALIGN(8);
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
		. = ALIGN(8);
	}
	.kernel_ex : {
		*(.text .text*)
		. = ALIGN(8);
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
		. = ALIGN(8);
	}
	.kernel_iw : {
		*(.idata .idata*)
		. = ALIGN(8);
	}
	.kernel_early_pointer : {
		*(.epointer)
		. = ALIGN(8);
	}
	/DISCARD/ : {
		*(.comment .gnu .gnu.* .note .note.*)
	}
}
