/* set_obj_thd (i960 0x5120; Gems 80038308): as set_obj_tpd, but g2 replaces
 * the model entry's second word (the texture header address) instead of the
 * first.
 *
 * Gems ends in the GameCube renderer (FUN_8002FA28); this is the i960's tail
 * instead (0x5174-0x51EC, then 0x5008). Gems also skips the polygon budget
 * the ROM tests first; it is kept. */
#pragma once
#include "set_obj.h"

static uint32_t gfn_set_obj_thd(int entry)
{
    uint32_t r[16];
    gso_frame_load(entry, r);
    if (gso_budget(r)) {
        gso_mov_matrix(r);
        if (!gso_model_ok(GEMS_G(0), r)) return gso_bad_model(entry, r, 0x4CB4, "set_obj_thd");
        gso_take_entry(r);
        r[9] = GEMS_G(2);
        gso_send_entry(r);
    }
    if (entry) gems_i960_ret();
    return 0;
}
