/* Included by m2-hle2's src/core/gems.h when built with -DM2HLE_GEMS_DIR. */
#pragma once
#ifndef GEMS_INNER   /* an m2-hle2 from before gems_inner: no profile hook is served */
typedef int (*gems_inner_fn)(i960_cpu_t *cpu, memory_bus_t *bus);
static inline gems_inner_fn gems_inner(uint32_t ip) { (void)ip; return NULL; }
static inline void gems_inner_call(gems_inner_fn f) { (void)f; }
#endif
#include "fn_protos.h"
#include "gems_all.h"   /* gen_all.py */
