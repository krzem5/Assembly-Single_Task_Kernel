ENTRY(_start)
SECTIONS{
	. = 0;
	.kernel_ue : {
		*(.entry)
		*(.etext .etext* .await_text .await_text*)
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
		PROVIDE_HIDDEN(__kernel_section_gcov_info_start = .);
		*(.gcov_info)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__kernel_section_gcov_info_end = .);
	}
	.kernel_rw : {
		*(.data .data* .bss .bss*)
	}
	.kernel_iw : {
		*(.idata .idata*)
	}
	.kernel_early_pointer : {
		*(.epointer)
	}
	.kernel_await : {
		*(.init_array)
	}
	.kernel_no_await : {
		*(.fini_array)
	}
	.text : { /* prevent GDB from crashing */
		. += 1;
	}
	/DISCARD/ : {
		*(.comment .gnu .gnu.* .note .note.*)
	}
}
