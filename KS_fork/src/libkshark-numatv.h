//NOTE: Changed here. (NUMA TV) (2025-04-19)
/* Copyright (C) 2025, David Jaromír Šebánek <djsebofficial@gmail.com> */

#ifndef _KSHARK_NUMA_TV_H
#define _KSHARK_NUMA_TV_H

// KernelShark
#include "libkshark.h"

#ifdef __cplusplus
extern "C" {
#endif

const char* numatv_get_topo_fpath(int stream_id);
int numatv_get_topo_view(int stream_id);

#ifdef __cplusplus
}
#endif

#endif // _KSHARK_NUMA_TV_H
// END of change