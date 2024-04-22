#ifndef _COMMON_KFS2_UTIL_H_
#define _COMMON_KFS2_UTIL_H_ 1
#if BUILD_MODULE
#include <common/kfs2/platform/module.h>
#elif BUILD_UEFI
#include <common/kfs2/platform/uefi.h>
#else
#include <common/kfs2/platform/tool.h>
#endif
#endif
