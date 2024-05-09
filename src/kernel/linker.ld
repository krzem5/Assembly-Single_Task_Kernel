ENTRY(_start)
SECTIONS{
	. = 0;
	.kernel_ue : {
		*(.entry)
		*(.etext .etext*)
		. = ALIGN(4096);
	}
	.kernel_ur : {
		*(.ap_startup)
		. = ALIGN(8);
		*(.erdata .erdata*)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_cpu_local_start = .);
		*(.cpulocal)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_cpu_local_end = .);
		PROVIDE_HIDDEN(__kernel_section_early_early_initializers_start = .);
		*(.eeinitializer)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_early_early_initializers_end = .);
		PROVIDE_HIDDEN(__kernel_section_early_initializers_start = .);
		*(.einitializer)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_early_initializers_end = .);
		PROVIDE_HIDDEN(__kernel_section_initializers_start = .);
		*(.initializer)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_initializers_end = .);
		. = ALIGN(4096);
	}
	.kernel_uw : {
		*(.edata .edata*)
		. = ALIGN(4096);
	}
	.kernel_ex : {
		*(.text .text*)
		. = ALIGN(4096);
	}
	.kernel_nx : {
		*(.rodata .rodata* .rdata .rdata*)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_gcov_info_start = .);
		*(.gcov_info)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_gcov_info_end = .);
		. = ALIGN(4096);
	}
	.kernel_rw : {
		*(.data .data* .bss .bss*)
		. = ALIGN(4096);
	}
	.kernel_iw : {
		*(.idata .idata*)
		. = ALIGN(4096);
	}
	.kernel_early_pointer : {
		*(.epointer)
		. = ALIGN(8);
	}
	/DISCARD/ : {
		*(.comment .gnu .gnu.* .note .note.*)
	}
}
