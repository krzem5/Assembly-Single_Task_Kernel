#ifndef _COMMON_FAT32_UTIL_H_
#define _COMMON_FAT32_UTIL_H_ 1
#include <common/types.h>
#if BUILD_MODULE
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#elif BUILD_UEFI
#include <common/platform/uefi.h>
#else
#include <common/platform/tool.h>
#endif
#endif
