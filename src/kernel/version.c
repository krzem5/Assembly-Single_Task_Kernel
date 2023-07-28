#include <kernel/_version.h>
#include <kernel/types.h>



const u64 __attribute__((section(".cversion"))) __core_version=KERNEL_VERSION;
const u64 __attribute__((section(".version"))) __version=KERNEL_VERSION;
