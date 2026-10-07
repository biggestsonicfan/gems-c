/* set_obj_fifo (i960 0x52C8; Gems 800383F0): draw model g0 under the 12-word
 * matrix waiting in the COP's reply FIFO (Fn_osage's segment matrices,
 * os_set_osage_after). The matrix goes to the display list (GEO 0xB0B) and to
 * g3's buffer, unless g2 is 1, the polygon budget is spent, the matrix's
 * first word is over 1.0 or its last under 0.2 in size; then the model, with
 * its texture-header word replaced by 0x800000 or 0x802000 for the fighter
 * flags at g7+0x1A4 / +0x70C / +0x1AA.
 *
 * Gems ends in the GameCube renderer (FUN_8002F898); this is the i960's tail
 * instead (0x5350-0x53C8, then 0x51B4 or 0x4F94, then 0x5008). */
#pragma once
#include "set_obj.h"

static uint32_t gfn_set_obj_fifo(int entry)
{
    uint32_t r[16];
    gso_frame_load(entry, r);
    gems_cop_rn(&r[4], 4);
    gems_cop_rn(&r[8], 4);
    gems_cop_rn(&r[12], 4);

    if (GEMS_G(2) == 1) { GSO_SET_CC(2); goto done; }
    GEMS_G(2) = gems_ld32(0x501018);
    r[3] = gems_ld32(0x50101C);
    if (r[3] > GEMS_G(2)) { GSO_SET_CC(1); goto done; }
    r[3] = 0x3F800000u;                               /* |m[0]| > 1.0 */
    GEMS_G(4) = r[4] & 0x7FFFFFFFu;
    if (r[3] < GEMS_G(4)) { GSO_SET_CC(4); goto done; }
    r[3] = 0x3E4CCCCDu;                               /* |m[11]| < 0.2 */
    GEMS_G(4) = r[15] & 0x7FFFFFFFu;
    if (r[3] > GEMS_G(4)) { GSO_SET_CC(1); goto done; }

    gso_geo_cmd(0xB0, 0xB0B);
    gems_stn(GEMS_G(10) + GEMS_G(12), &r[4], 4);
    gems_stn(GEMS_G(10) + GEMS_G(12), &r[8], 4);
    gems_stn(GEMS_G(10) + GEMS_G(12), &r[12], 4);
    for (int q = 4; q < 16; q += 4) {
        gems_stn(GEMS_G(3), &r[q], 4);
        GEMS_G(3) += 16;
    }

    /* 0x53D0 takes g3 back the 0x30 and goes to the err poly screen */
    if (!gso_model_ok(GEMS_G(0), r)) return gso_bad_model(entry, r, 0x53D0, "set_obj_fifo");

    bool header = false;
    r[15] = gems_ld32(GEMS_G(7) + 0x1A4);
    if (r[15] & (1u << 26)) {
        r[15] = gems_ld16(GEMS_G(7) + 0x1AA);
        if (r[15] <= 4) { GEMS_G(2) = 0x800000u; header = true; }
    }
    if (!header) {
        r[15] = gems_ld32(GEMS_G(7) + 0x70C);
        if (r[15] & 4u) {
            r[15] = gems_ld16(GEMS_G(7) + 0x1AA);
            if (r[15] <= 4 && (r[15] & 1u)) { GEMS_G(2) = 0x802000u; header = true; }
        }
    }
    if (header) {
        gso_take_entry(r);                            /* 0x51B4 */
        r[9] = GEMS_G(2);
    } else {
        gso_model_ok(GEMS_G(0), r);                   /* 0x4F94 checks again; it passes */
        gso_take_entry(r);
    }
    gso_send_entry(r);
done:
    if (entry) gems_i960_ret();
    return 0;
}
