/* zanzou_disp (i960 0x8ACD0, Gems 0x8004B434): draw the afterimage ring.
 * For each of the 128 slots, COP 0x85 (Fn_zanzou_get) answers five words;
 * a live slot (third word nonzero) is drawn on every other frame (slot +
 * frame counter odd): push the matrix (0x01), mirror X when g3 is set (0x07
 * scale -1 in X; Gems' entry sets g3 = 0), load the slot's matrix (0x84),
 * turn by rob+0x5C of the fighter at [0x5008A4] (0x08, ang_x), draw the
 * object the fifth word names, pop (0x02). */
#pragma once

static uint32_t gfn_zanzou_disp(int entry)
{
    GEMS_G(3) = 0;
    uint32_t rob = gems_ld32(0x5008A4);
    for (uint32_t slot = 0; slot < 0x80; slot++) {
        uint32_t rep[5];
        gems_cop_w(0x42808585);
        gems_cop_w(slot);
        gems_cop_rn(rep, 5);
        uint32_t live = rep[2], obj = rep[4];
        if (live == 0) continue;
        if (!((gems_ld32(0x500020) + slot) & 1)) continue;
        gems_cop_w(0x00800101);
        if (GEMS_G(3) != 0) {
            gems_cop_w(0x03800707);
            gems_cop_w(0x3F800000);
            gems_cop_w(0xBF800000);
            gems_cop_w(0x3F800000);
        }
        gems_cop_w(0x42008484);
        gems_cop_w(slot);
        gems_cop_w(0x04000808);
        gems_cop_w((uint32_t)gems_ld16s(rob + 0x5C));   /* ldis */
        GEMS_G(1) = 0;
        GEMS_G(0) = obj;
        gfn_set_obj();
        gems_cop_w(0x01000202);
    }
    if (entry) gems_i960_ret();
    return 0;
}
