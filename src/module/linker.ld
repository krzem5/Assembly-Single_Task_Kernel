SECTIONS{
	. = 0;
	.module_ue : {
		*(.etext .etext.*)
	}
	. = ALIGN(4096);
	.module_ur : {
		*(.erdata .erdata.*)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__module_preinit_start = .);
		*(.module_preinit)
		PROVIDE_HIDDEN(__module_preinit_end = .);
		. = ALIGN(8);
		PROVIDE_HIDDEN(__module_init_start = .);
		*(.module_init)
		PROVIDE_HIDDEN(__module_init_end = .);
		. = ALIGN(8);
		PROVIDE_HIDDEN(__module_postinit_start = .);
		*(.module_postinit)
		PROVIDE_HIDDEN(__module_postinit_end = .);
		. = ALIGN(8);
		PROVIDE_HIDDEN(__module_postpostinit_start = .);
		*(.module_postpostinit)
		PROVIDE_HIDDEN(__module_postpostinit_end = .);
	}
	. = ALIGN(4096);
	.module_uw : {
		*(.edata .edata.*)
	}
	. = ALIGN(4096);
	.module_ex : {
		*(.text .text.*)
	}
	. = ALIGN(4096);
	.module_nx : {
		*(.rodata .rodata.*)
		. = ALIGN(8);
		PROVIDE_HIDDEN(__module_deinit_start = .);
		*(.module_deinit)
		PROVIDE_HIDDEN(__module_deinit_end = .);
		. = ALIGN(8);
		PROVIDE_HIDDEN(__module_gcov_info_start = .);
		*(.gcov_info)
		PROVIDE_HIDDEN(__module_gcov_info_end = .);
	}
	. = ALIGN(4096);
	.module_rw : {
		*(.data .data.*)
	}
	. = ALIGN(4096);
	.module_iw : {
		*(.idata .idata.*)
	}
	. = ALIGN(4096);
	.module_zw : {
		*(.bss .bss.*)
	}
	. = ALIGN(4096);
	.signature : {
		*(.signature)
		. = ALIGN(4096);
	}
	/DISCARD/ : {
		*(.comment .gnu .gnu.* .note .note.*)
	}
}
