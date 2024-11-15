OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)
SECTIONS{
	. = 0;
	_IMAGE_BASE = .;
	.gnu.hash : {
		*(.gnu.hash)
	}
	. = ALIGN(4096);
	.text : {
		*(.text .text*)
		. = ALIGN(16);
	}
	. = ALIGN(4096);
	.reloc : {
		*(.reloc)
	}
	. = ALIGN(4096);
	.data : {
		*(.got .got.plt)
		*(.rodata .rodata* .data .data* .sdata .sbss .scommon .dynbss .bss COMMON)
		*(.rel.local)
	}
	. = ALIGN(4096);
	.dynamic : {
		*(.dynamic)
	}
	. = ALIGN(4096);
	.rela : {
		*(.rela .rela*)
	}
	. = ALIGN(4096);
	.dynsym : {
		*(.dynsym)
	}
	. = ALIGN(4096);
	.dynstr : {
		*(.dynstr)
	}
	. = ALIGN(4096);
	/DISCARD/ : {
		*(.note.* .gnu_debuglink .gnu.lto_* .comment .hash*)
	}
}
