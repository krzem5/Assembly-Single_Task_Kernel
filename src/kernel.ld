ENTRY(_start)
SECTIONS{
    . = 0xffffffffc0100000;
    __KERNEL_OFFSET__ = 0xffffffffc0000000;
    __KERNEL_START__ = . - __KERNEL_OFFSET__;
    .text : AT(ADDR(.text) - __KERNEL_OFFSET__) {
        *(.entry)
        *(.version.core)
        *.core.*(.text .text*)
        *.core.*(.rodata .rodata* .rdata .rdata*)
        . = ALIGN(0x1000);
    }
    .data : AT(ADDR(.data) - __KERNEL_OFFSET__) {
        *.core.*(.data .data*)
        *.core.*(.bss .bss*)
        . = ALIGN(0x1000);
    }
    __KERNEL_LOW_END__ = . - __KERNEL_OFFSET__;
    .text2 : AT(ADDR(.text2) - __KERNEL_OFFSET__){
        ASSERT (__KERNEL_LOW_END__ - __KERNEL_START__ <= 65536, "Kernel core too large");
        *(.version)
        *(.text .text*)
        *(.rodata .rodata* .rdata .rdata*)
        . = ALIGN(0x1000);
    }
    .data2 : AT(ADDR(.data2) - __KERNEL_OFFSET__){
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
