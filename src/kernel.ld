ENTRY(_start)
SECTIONS{
    . = 0xffffffffc0100000;
    __KERNEL_OFFSET__ = 0xffffffffc0000000;
    __KERNEL_START__ = . - __KERNEL_OFFSET__;
    .core : AT(ADDR(.core) - __KERNEL_OFFSET__){
        *(.entry)
        *(.cversion)
        *(.ctext .ctext*)
        *(.crdata .crdata*)
       	*(.cdata .cdata*)
        . = ALIGN(0x1000);
    }
    __KERNEL_LOW_END__ = . - __KERNEL_OFFSET__;
    .text : AT(ADDR(.text) - __KERNEL_OFFSET__){
        ASSERT (__KERNEL_LOW_END__ - __KERNEL_START__ <= 65536, "Kernel core too large");
        *(.version)
        *(.text .text*)
        *(.rodata .rodata* .rdata .rdata*)
        . = ALIGN(0x1000);
    }
    .data : AT(ADDR(.data) - __KERNEL_OFFSET__){
        *(.data .data*)
        *(.bss .bss*)
        *(.ap_startup)
        . = ALIGN(0x1000);
    }
    __KERNEL_COMMON_START__ = . - __KERNEL_OFFSET__;
    .common : AT(ADDR(.common) - __KERNEL_OFFSET__){
        *(.common)
        . = ALIGN(0x1000);
    }
    __KERNEL_END__ = . - __KERNEL_OFFSET__;
    /DISCARD/ : {
        *(*)
    }
}
