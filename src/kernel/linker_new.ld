#define DECLARE_SECTION_START(name) __KERNEL_SECTION_##name##_START__ = .
#define DECLARE_SECTION_END(name) __KERNEL_SECTION_##name##_END__ = .

#define BASE_ADDRESS_OFFSET 0xffffffffc0000000



ENTRY(main)
SECTIONS{
    . = 0xffffffffc0100000;
    DECLARE_SECTION_START(address_range);
    DECLARE_SECTION_START(core);
    DECLARE_SECTION_START(core_ex);
    .core_ex : AT(ADDR(.core_ex) - BASE_ADDRESS_OFFSET){
        *(.entry)
        *(.cversion .version)
        *(.ctext .ctext* .text .text*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(core_ex);
    DECLARE_SECTION_START(core_nx);
    .core_nx : AT(ADDR(.core_nx) - BASE_ADDRESS_OFFSET){
        *(.crdata .crdata* .rodata .rodata* .rdata .rdata*)
        . = ALIGN(8);
	    DECLARE_SECTION_START(pmm_counter);
        *(.pmmcounter*)
        . = ALIGN(8);
	    DECLARE_SECTION_END(pmm_counter);
	    DECLARE_SECTION_START(cpu_local);
        *(.cpulocal)
        . = ALIGN(8);
	    DECLARE_SECTION_END(cpu_local);
	    DECLARE_SECTION_START(handle);
        *(.handle)
        . = ALIGN(8);
	    DECLARE_SECTION_END(handle);
	    DECLARE_SECTION_START(sandbox);
        *(.sandbox)
        . = ALIGN(8);
	    DECLARE_SECTION_END(sandbox);
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
    DECLARE_SECTION_END(core_nx);
    DECLARE_SECTION_START(core_rw);
    .core_rw : AT(ADDR(.core_rw) - BASE_ADDRESS_OFFSET){
        *(.cdata .cdata* .data .data*)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(core_rw);
    DECLARE_SECTION_END(core);
    DECLARE_SECTION_START(bss);
    .bss : AT(ADDR(.bss) - BASE_ADDRESS_OFFSET){
        *(.cbss)
        *(.bss)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(bss);
    DECLARE_SECTION_END(address_range);
    DECLARE_SECTION_START(relocaions);
    .got : AT(ADDR(.got) - BASE_ADDRESS_OFFSET){
        *(.got .got*)
        . = ALIGN(0x1000);
    }
    .reloc : AT(ADDR(.reloc) - BASE_ADDRESS_OFFSET){
        *(.reloc)
        . = ALIGN(0x1000);
    }
    DECLARE_SECTION_END(relocaions);
#ifndef _KERNEL_DEBUG_BUILD_
    /DISCARD/ : {
        *(*)
    }
#endif
}
