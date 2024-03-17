SEARCH_DIR("=build/lib");
ENTRY(_start)
SECTIONS{
	. = SIZEOF_HEADERS;
	.hash : {
		*(.hash)
	}
	.dynsym : {
		*(.dynsym)
	}
	.dynstr : {
		*(.dynstr)
	}
	.rela.plt : {
		*(.rela.plt)
	}
	.rela.dyn : {
		*(.rela .rela.*)
	}
	. = ALIGN(4096);
	.init : {
		KEEP(*(SORT_NONE(.init)))
	}
	.plt : {
		*(.plt)
	}
	.plt.got : {
		*(.plt.got)
	}
	.text : {
		*(.text .text.*)
	}
	.fini : {
		KEEP(*(SORT_NONE(.fini)))
	}
	. = ALIGN(4096);
	.rodata : {
		*(.rodata .rodata.*)
	}
	.eh_frame : ONLY_IF_RO {
		KEEP(*(.eh_frame))
		*(.eh_frame.*)
	}
	. = ALIGN(4096);
	.eh_frame : ONLY_IF_RW {
		KEEP(*(.eh_frame))
		*(.eh_frame.*)
	}
	.preinit_array : {
		PROVIDE_HIDDEN(__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN(__preinit_array_end = .);
	}
	.init_array : {
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*(.init_array))
		PROVIDE_HIDDEN(__init_array_end = .);
	}
	.fini_array : {
		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN(__fini_array_end = .);
	}
	.ctors : {
		KEEP(*(.ctors))
		KEEP(*(SORT(.ctors.*)))
	}
	.dtors : {
		KEEP(*(.dtors))
		KEEP(*(SORT(.dtors.*)))
	}
	.dynamic : {
		*(.dynamic)
	}
	.got : {
		*(.got)
	}
	. = ALIGN(SIZEOF(.got.plt)>=24?24:0);
	.got.plt : {
		*(.got.plt)
	}
	.data : {
		*(.data .data.*)
		SORT(CONSTRUCTORS)
	}
	.gcov_info : {
		PROVIDE_HIDDEN(__gcov_info_start = .);
		*(.gcov_info .gcov_info.*)
		PROVIDE_HIDDEN(__gcov_info_end = .);
	}
	.bss : {
		*(.bss .bss.* .dynbss)
		*(COMMON)
		. = ALIGN(8);
	}
	.signature : {
		KEEP(*(.signature))
	}
	.debug_abbrev 0 : {
		*(.debug_abbrev)
	}
	.debug_addr 0 : {
		*(.debug_addr)
	}
	.debug_aranges 0 : {
		*(.debug_aranges)
	}
	.debug_frame 0 : {
		*(.debug_frame)
	}
	.debug_funcnames 0 : {
		*(.debug_funcnames)
	}
	.debug_info 0 : {
		*(.debug_info)
	}
	.debug_line 0 : {
		*(.debug_line .debug_line.* .debug_line_end)
	}
	.debug_line_str 0 : {
		*(.debug_line_str)
	}
	.debug_loc 0 : {
		*(.debug_loc)
	}
	.debug_loclists 0 : {
		*(.debug_loclists)
	}
	.debug_macinfo 0 : {
		*(.debug_macinfo)
	}
	.debug_macro 0 : {
		*(.debug_macro)
	}
	.debug_names 0 : {
		*(.debug_names)
	}
	.debug_pubnames 0 : {
		*(.debug_pubnames)
	}
	.debug_pubtypes 0 : {
		*(.debug_pubtypes)
	}
	.debug_ranges 0 : {
		*(.debug_ranges)
	}
	.debug_rnglists 0 : {
		*(.debug_rnglists)
	}
	.debug_str 0 : {
		*(.debug_str)
	}
	.debug_str_offsets 0 : {
		*(.debug_str_offsets)
	}
	.debug_sup 0 : {
		*(.debug_sup)
	}
	.debug_typenames 0 : {
		*(.debug_typenames)
	}
	.debug_varnames 0 : {
		*(.debug_varnames)
	}
	.debug_weaknames 0 : {
		*(.debug_weaknames)
	}
	/DISCARD/ : {
		*(.note.* .gnu_debuglink .gnu.lto_* .comment)
	}
}
