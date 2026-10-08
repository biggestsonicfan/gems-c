/*
 * gems_cop_replay: m2-hle2's tests/cop_replay.c with the COP commands answered
 * by this repository's handlers (sharc/) instead of sharc_exec, as --gems-cop
 * does live. It replays a MAME capture of the firmware's FIFOs
 * (m2-hle2 tools/mame/cop-capture.lua) and grades every reply word against
 * what the firmware wrote: per command, exact / close / wrong.
 *
 * Commands the Gems C leaves empty, and Fn_put_poly (0x78), go to sharc_exec,
 * as live. After each Gems command the current matrix is copied from DM to
 * g_sharc.rot / pos (gems.h's gems_cop_sync_matrix), so cop_replay's matrix
 * checks see Gems' state; with RESYNC=1 the board's matrix, which cop_replay
 * puts there, is copied back into DM before the next Gems command.
 *
 * Build: tools/cop_replay/build.sh <m2-hle2 checkout> [out]
 * Run:   gems_cop_replay <capture prefix> [examples-per-op] [only-op-hex]
 */
#include <stdbool.h>
#include "cop.h"

/* ---- gems.h's COP runtime: the part sharc/ uses --------------------------- */
typedef struct { const uint32_t *in; int in_n, in_i; uint32_t cmd; } gems_rt_t;
static gems_rt_t g_gems;

static inline float    gems_u2f(uint32_t u) { float f; memcpy(&f, &u, 4); return f; }
static inline uint32_t gems_f2u(float f)    { uint32_t u; memcpy(&u, &f, 4); return u; }
static inline uint32_t gems_in_w(void) {
    if (g_gems.in_i < g_gems.in_n) return g_gems.in[g_gems.in_i++];
    g_gems.in_i++;
    return 0;
}
static inline float gems_in_f(void) { return gems_u2f(gems_in_w()); }
static inline void  gems_out_w(uint32_t v) { sharc_push_u(v); }
static inline void  gems_out_f(float f)    { sharc_push_u(gems_f2u(f)); }
#define GEMS_DM_BASE  0x30000u
#define GEMS_DM_WORDS ((uint32_t)(sizeof g_sharc.dm / sizeof g_sharc.dm[0]))
static inline uint32_t *gems_dm(uint32_t addr) {
    static uint32_t junk;
    uint32_t i = addr - GEMS_DM_BASE;
    if (i >= GEMS_DM_WORDS) { junk = 0; return &junk; }
    return &g_sharc.dm[i];
}
static inline float *gems_dmf(uint32_t addr) { return (float *)(void *)gems_dm(addr); }
static inline uint32_t gems_bram_rd(uint32_t idx) {
    uint32_t v = 0;
    if (g_sharc.sharc_dm_ext)
        memcpy(&v, g_sharc.sharc_dm_ext + ((idx * 4u) & (g_sharc.sharc_dm_ext_size - 1u)), 4);
    return v;
}
static inline void gems_bram_wr(uint32_t idx, uint32_t v) {
    if (g_sharc.sharc_dm_ext)
        memcpy(g_sharc.sharc_dm_ext + ((idx * 4u) & (g_sharc.sharc_dm_ext_size - 1u)), &v, 4);
}
static inline float gems_bram_rdf(uint32_t idx)          { return gems_u2f(gems_bram_rd(idx)); }
static inline void  gems_bram_wrf(uint32_t idx, float f) { gems_bram_wr(idx, gems_f2u(f)); }

typedef struct { uint32_t site; uint32_t (*fn)(int entry); const char *name; } gems_trap_t;
typedef struct { void (*fn)(void); int args; const char *name; } gems_cop_op_t;
#define GEMS_COP_OPS 0x88
#include "gems_cop_all.h"   /* gen_all.py --cop-only */

/* ---- The replay's sharc_exec ------------------------------------------------ */
static float *gems_cur_matrix(void) { return gems_dmf(GEMS_DM_BASE + *gems_dm(GEMS_DM_BASE + 0xCFCu / 4u)); }

/* g_sharc.rot / pos (sharc_exec's matrix) from and into Gems' current one */
static void gems_matrix_to_sharc(const float *m) {
    for (int c = 0; c < 3; c++) for (int r = 0; r < 3; r++) g_sharc.rot[c][r] = m[c * 3 + r];
    for (int r = 0; r < 3; r++) g_sharc.pos[r] = m[9 + r];
}
static void gems_matrix_from_sharc(float *m) {
    for (int c = 0; c < 3; c++) for (int r = 0; r < 3; r++) m[c * 3 + r] = g_sharc.rot[c][r];
    for (int r = 0; r < 3; r++) m[9 + r] = g_sharc.pos[r];
}

static void gems_replay_exec(uint32_t cmd, const uint32_t *args, int n) {
    uint32_t op = (cmd >> 23) & 0x1FFu;
    bool mine = op < GEMS_COP_OPS && cmd == ((op << 23) | (op << 8) | op)
                && op != 0x78 && gems_cop_ops[op].fn;
    if (!mine) {
        if (op == 0x78) gems_matrix_to_sharc(gems_cur_matrix());   /* our Fn_put_poly reads g_sharc */
        sharc_exec(cmd, args, n);
        return;
    }
    static int resync = -1;
    if (resync < 0) resync = getenv("RESYNC") != NULL;
    if (resync) gems_matrix_from_sharc(gems_cur_matrix());
    g_sharc.reply_count = 0;
    g_sharc.reply_idx   = 0;
    g_gems.cmd = cmd; g_gems.in = args; g_gems.in_n = n; g_gems.in_i = 0;
    gems_cop_ops[op].fn();
    gems_matrix_to_sharc(gems_cur_matrix());
}

/* cop_reset() (in cop_replay's main) runs the firmware's DM init: Gems' here. */
__attribute__((constructor)) static void gems_replay_init(void) { g_gems_cop_reset = gems_cop_impl_reset; }

#define sharc_exec gems_replay_exec
#include "cop_replay.c"
