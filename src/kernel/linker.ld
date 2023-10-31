#define DECLARE_SECTION_START(name) __KERNEL_SECTION_##name##_START__ = .
#define DECLARE_SECTION_END(name) __KERNEL_SECTION_##name##_END__ = .

#define BASE_ADDRESS_OFFSET 0xffffffffc0000000



ENTRY(_start)
SECTIONS{
    . = 0xffffffffc0100000;
    DECLARE_SECTION_START(kernel);
    DECLARE_SECTION_START(kernel_ex);
    .kernel_ex : AT(ADDR(.kernel_ex) - BASE_ADDRESS_OFFSET){
        *(.entry)
        *(.text .text*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_ex);
    DECLARE_SECTION_START(kernel_nx);
    .kernel_nx : AT(ADDR(.kernel_nx) - BASE_ADDRESS_OFFSET){
        *(.rodata .rodata* .rdata .rdata*)
        . = ALIGN(8);
	    DECLARE_SECTION_START(tmp_init);
        *(.tmp_init)
        . = ALIGN(8);
	    DECLARE_SECTION_END(tmp_init);
	    DECLARE_SECTION_START(cpu_local);
        *(.cpulocal)
        . = ALIGN(8);
	    DECLARE_SECTION_END(cpu_local);
	    DECLARE_SECTION_START(handle);
        *(.handle)
        . = ALIGN(8);
	    DECLARE_SECTION_END(handle);
#ifdef _KERNEL_COVERAGE_BUILD_
	    DECLARE_SECTION_START(gcov_info);
        *(.gcov_info)
        . = ALIGN(8);
	    DECLARE_SECTION_END(gcov_info);
#endif
        . = ALIGN(0x1000);
        *(.ap_startup)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_nx);
    DECLARE_SECTION_START(kernel_rw);
    .kernel_rw : AT(ADDR(.kernel_rw) - BASE_ADDRESS_OFFSET){
        *(.data .data*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_rw);
    DECLARE_SECTION_START(kernel_iw);
    .kernel_iw : AT(ADDR(.kernel_iw) - BASE_ADDRESS_OFFSET){
        *(.idata .idata*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_iw);
    DECLARE_SECTION_START(kernel_bss);
    .bss : AT(ADDR(.bss) - BASE_ADDRESS_OFFSET){
        *(.bss)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_bss);
    DECLARE_SECTION_END(kernel);
#ifndef _KERNEL_DEBUG_BUILD_
    /DISCARD/ : {
        *(*)
    }
#endif
}
