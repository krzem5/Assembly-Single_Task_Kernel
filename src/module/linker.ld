SECTIONS{
	. = 0;
	.text : {
		*(.text .text.*)
	}
	. = ALIGN(4096);
	.rodata : {
		*(.rodata .rodata.*)
	}
	.gcov_info : {
		*(.gcov_info)
	}
	.module : {
		*(.module)
	}
	. = ALIGN(4096);
	.data : {
		*(.data .data.*)
	}
	. = ALIGN(4096);
	.bss : {
		*(.bss .bss.*)
	}
	. = ALIGN(4096);
	.signature : {
		*(.signature)
		. = ALIGN(1024);
	}
	/DISCARD/ : {
		*(.comment .gnu .gnu.* .note .note.*)
	}
}
