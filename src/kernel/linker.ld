ENTRY(main)
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
        . = ALIGN(8);
        __KERNEL_PMM_COUNTER_START__ = .;
        *(.pmmcounter)
        __KERNEL_PMM_COUNTER_END__ = .;
        . = ALIGN(0x1000);
    }
    __KERNEL_CORE_END__ = . - __KERNEL_OFFSET__;
    .kernel : AT(ADDR(.kernel) - __KERNEL_OFFSET__){
        ASSERT(__KERNEL_CORE_END__ - __KERNEL_START__ <= 65536, "Kernel core too large");
        *(.version)
        *(.text .text*)
        *(.rodata .rodata* .rdata .rdata*)
        *(.data .data*)
        . = ALIGN(8);
        __KERNEL_CPU_LOCAL_START__ = .;
        *(.cpulocal)
        __KERNEL_CPU_LOCAL_END__ = .;
        . = ALIGN(8);
        __KERNEL_HANDLE_START__ = .;
        *(.handle)
        __KERNEL_HANDLE_END__ = .;
        . = ALIGN(8);
        __KERNEL_SANDBOX_START__ = .;
        *(.sandbox)
        __KERNEL_SANDBOX_END__ = .;
#ifdef _KERNEL_COVERAGE_BUILD_
        . = ALIGN(8);
        __KERNEL_GCOV_INFO_START__ = .;
        *(.gcov_info)
        __KERNEL_GCOV_INFO_END__ = .;
#endif
        . = ALIGN(0x1000);
        *(.ap_startup)
        . = ALIGN(0x1000);
    }
    __KERNEL_COMMON_START__ = . - __KERNEL_OFFSET__;
    .common : AT(ADDR(.common) - __KERNEL_OFFSET__){
        *(.common)
        . = ALIGN(0x1000);
    }
    __KERNEL_END__ = . - __KERNEL_OFFSET__;
    __KERNEL_BSS_START__ = . - __KERNEL_OFFSET__;
    .bss : AT(ADDR(.bss) - __KERNEL_OFFSET__){
        *(.cbss)
        *(.bss)
        . = ALIGN(0x1000);
    }
    __KERNEL_BSS_END__ = . - __KERNEL_OFFSET__;
#ifndef _KERNEL_DEBUG_BUILD_
    /DISCARD/ : {
        *(*)
    }
#endif
}
