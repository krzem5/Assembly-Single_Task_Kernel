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
	u8 boot_fs_guid[16];
	u8 master_key[64];
	struct{
		u16 year;
		u8 month;
		u8 day;
		u8 hour;
		u8 minute;
		u8 second;
		u32 nanosecond;
		u64 measurement_offset;
	} date;
} kernel_data_t;



typedef void (*kernel_initializer_t)(void);



extern const u64 _version;
extern const char* _build_name;

extern kernel_data_t kernel_data;



static KERNEL_INLINE u64 KERNEL_NOCOVERAGE kernel_get_version(void){
	return _version;
}



static KERNEL_INLINE const char* KERNEL_NOCOVERAGE kernel_get_build_name(void){
	return _build_name;
}



static KERNEL_INLINE u64 KERNEL_NOCOVERAGE kernel_get_offset(void){
	return 0xffffffffc0000000ull;
}



_KERNEL_DECLARE_SECTION(kernel);
_KERNEL_DECLARE_SECTION(kernel_ue);
_KERNEL_DECLARE_SECTION(kernel_ur);
_KERNEL_DECLARE_SECTION(kernel_uw);
_KERNEL_DECLARE_SECTION(kernel_ex);
_KERNEL_DECLARE_SECTION(kernel_nx);
_KERNEL_DECLARE_SECTION(kernel_rw);
_KERNEL_DECLARE_SECTION(kernel_iw);

_KERNEL_DECLARE_SECTION(cpu_local);
_KERNEL_DECLARE_SECTION(early_early_initializers);
_KERNEL_DECLARE_SECTION(early_initializers);
_KERNEL_DECLARE_SECTION(initializers);
_KERNEL_DECLARE_SECTION(gcov_info);



void kernel_init(const kernel_data_t* bootloader_kernel_data);



void kernel_adjust_memory_flags(void);



void kernel_early_execute_initializers(void);



void kernel_execute_initializers(void);



void kernel_adjust_memory_flags_after_init(void);



u64 kernel_gcov_info_data(u64* size);



#endif
