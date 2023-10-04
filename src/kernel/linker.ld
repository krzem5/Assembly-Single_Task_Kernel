#define DECLARE_SECTION_START(name) __KERNEL_SECTION_##name##_START__ = .;
#define DECLARE_SECTION_END(name) __KERNEL_SECTION_##name##_END__ = .;



ENTRY(main)
SECTIONS{
    . = 0xffffffffc0100000;
    __KERNEL_OFFSET__ = 0xffffffffc0000000;
    DECLARE_SECTION_START(address_range)
    DECLARE_SECTION_START(core)
    DECLARE_SECTION_START(core_ex)
    .core_ex : AT(ADDR(.core_ex) - __KERNEL_OFFSET__){
        *(.entry)
        *(.cversion)
        *(.ctext .ctext*)
    }
    DECLARE_SECTION_END(core_ex)
    DECLARE_SECTION_START(core_nx)
    .core_nx : AT(ADDR(.core_nx) - __KERNEL_OFFSET__){
        *(.crdata .crdata*)
        . = ALIGN(8);
	    DECLARE_SECTION_START(pmm_counter)
        *(.pmmcounter*)
	    DECLARE_SECTION_END(pmm_counter)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(core_nx)
    DECLARE_SECTION_START(core_rw)
    .core_rw : AT(ADDR(.core_rw) - __KERNEL_OFFSET__){
        *(.cdata .cdata*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(core_rw)
    DECLARE_SECTION_END(core)
    DECLARE_SECTION_START(kernel)
    DECLARE_SECTION_START(kernel_ex)
    .kernel_ex : AT(ADDR(.kernel_ex) - __KERNEL_OFFSET__){
        ASSERT(__KERNEL_SECTION_core_END__ - __KERNEL_SECTION_core_START__ <= 65536, "Kernel core too large");
        *(.version)
        *(.text .text*)
    }
    DECLARE_SECTION_END(kernel_ex)
    DECLARE_SECTION_START(kernel_nx)
    .kernel_nx : AT(ADDR(.kernel_nx) - __KERNEL_OFFSET__){
        *(.rodata .rodata* .rdata .rdata*)
        . = ALIGN(8);
	    DECLARE_SECTION_START(cpu_local)
        *(.cpulocal)
        . = ALIGN(8);
	    DECLARE_SECTION_END(cpu_local)
	    DECLARE_SECTION_START(handle)
        *(.handle)
        . = ALIGN(8);
	    DECLARE_SECTION_END(handle)
	    DECLARE_SECTION_START(sandbox)
        *(.sandbox)
        . = ALIGN(8);
	    DECLARE_SECTION_END(sandbox)
#ifdef _KERNEL_COVERAGE_BUILD_
	    DECLARE_SECTION_START(gcov_info)
        *(.gcov_info)
        . = ALIGN(8);
	    DECLARE_SECTION_END(gcov_info)
#endif
        . = ALIGN(0x1000);
        *(.ap_startup)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_nx)
    DECLARE_SECTION_START(kernel_rw)
    .kernel_rw : AT(ADDR(.kernel_rw) - __KERNEL_OFFSET__){
        *(.data .data*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(kernel_rw)
    DECLARE_SECTION_START(common)
    DECLARE_SECTION_START(common_ex)
    .common_ex : AT(ADDR(.common_ex) - __KERNEL_OFFSET__){
        *(.common)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(common_ex)
    DECLARE_SECTION_START(common_rw)
    .common_rw : AT(ADDR(.common_rw) - __KERNEL_OFFSET__){
        *(.common_rw)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(common_rw)
    DECLARE_SECTION_END(common)
    DECLARE_SECTION_END(kernel)
    DECLARE_SECTION_START(bss)
    .bss : AT(ADDR(.bss) - __KERNEL_OFFSET__){
        *(.cbss)
        *(.bss)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(bss)
    DECLARE_SECTION_END(address_range)
#ifndef _KERNEL_DEBUG_BUILD_
    /DISCARD/ : {
        *(*)
    }
#endif
}
