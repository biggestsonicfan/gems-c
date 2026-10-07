/* set_obj_tpd (i960 0x5050; Gems 800387C0): draw model g0 with replaced
 * texture points: the current matrix goes into the display list through COP
 * 0x34 (Fn_mov_matrix), then the model as object command 0x101 with its first
 * entry word replaced by g2, the texture-point address.
 *
 * Gems ends in the GameCube renderer (FUN_8002F978); this is the i960's tail
 * instead (0x50A4-0x511C, then 0x5008): the model check, the object count,
 * the entry from the ROM's model table, the polygon count and the GEO words.
 * Gems also skips the polygon budget the ROM tests first; it is kept. */
#pragma once
#include "set_obj.h"

static uint32_t gfn_set_obj_tpd(int entry)
{
    uint32_t r[16];
    gso_frame_load(entry, r);
    if (gso_budget(r)) {
        gso_mov_matrix(r);
        if (!gso_model_ok(GEMS_G(0), r)) return gso_bad_model(entry, r, 0x4CB4, "set_obj_tpd");
        gso_take_entry(r);
        r[8] = GEMS_G(2);
        gso_send_entry(r);
    }
    if (entry) gems_i960_ret();
    return 0;
}
