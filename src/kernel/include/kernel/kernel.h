#ifndef _KERNEL_KERNEL_H_
#define _KERNEL_KERNEL_H_ 1
#include <kernel/types.h>



#define _KERNEL_DECLARE_SECTION(name) \
	static KERNEL_INLINE u64 KERNEL_NOCOVERAGE kernel_section_##name##_start(void){ \
		extern u64 __KERNEL_SECTION_##name##_START__[1]; \
		return (u64)(__KERNEL_SECTION_##name##_START__);  \
	} \
	static KERNEL_INLINE u64 KERNEL_NOCOVERAGE kernel_section_##name##_end(void){ \
		extern u64 __KERNEL_SECTION_##name##_END__[1]; \
		return (u64)(__KERNEL_SECTION_##name##_END__);  \
	}



typedef struct _KERNEL_DATA{
	u16 mmap_size;
	struct{
		u64 base;
		u64 length;
	} mmap[42];
	u64 first_free_address;
	u64 rsdp_address;
	u64 smbios_address;
	u64 initramfs_address;
	u64 initramfs_size;
	u8 boot_fs_uuid[16];
} kernel_data_t;



extern const u64 __version;
extern const u64 kernel_symbols[];
extern kernel_data_t kernel_data;



static KERNEL_INLINE u64 KERNEL_NOCOVERAGE kernel_get_version(void){
	return __version;
}



static KERNEL_INLINE u64 KERNEL_NOCOVERAGE kernel_get_offset(void){
	return 0xffffffffc0000000ull;
}



_KERNEL_DECLARE_SECTION(kernel);
_KERNEL_DECLARE_SECTION(kernel_ex);
_KERNEL_DECLARE_SECTION(kernel_nx);
_KERNEL_DECLARE_SECTION(kernel_rw);
_KERNEL_DECLARE_SECTION(kernel_iw);
_KERNEL_DECLARE_SECTION(kernel_bss);

_KERNEL_DECLARE_SECTION(cpu_local);
_KERNEL_DECLARE_SECTION(handle);
_KERNEL_DECLARE_SECTION(gcov_info);



void kernel_init(const kernel_data_t* bootloader_kernel_data);



void kernel_adjust_memory_flags(void);



void kernel_adjust_memory_flags_after_init(void);



u64 kernel_gcov_info_data(u64* size);



#endif
