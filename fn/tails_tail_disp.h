/* tails_tail_disp (i960 0x1AB58, Gems 800474F4): Tails' two tails for the
 * fighter at g7. `tails` is the two-entry model table drawn while the
 * fighter is in a spin (rob+0x7F0 bit 16), `swing` the 64-entry table of
 * swinging tail models indexed by the counter at rob+0x2198. */

static uint32_t gfn_tails_tail_disp(int32_t entry, uint32_t tails, int32_t swing)
{
    uint32_t rob = GEMS_G(7);
    if (gems_ld8(rob + 0x84C) == 3) {
        if (entry) gems_i960_ret();
        return 0;
    }

    uint32_t phase = gems_ld16(rob + 0x2198);
    uint32_t next = phase + 1;
    if (next > 0x3F) next = 0;
    gems_st16(rob + 0x2198, next);

    if (gems_ld32(rob + 0x7F0) & 0x10000) {
        uint32_t odd = (gems_ld8(rob + 0x7F4) - 1u) & 1;
        uint32_t frame;
        gems_cop_w(0x00800101);                   /* push matrix */
        if (odd) {
            frame = gems_ld32(0x500020);
            gems_cop_w(0x03000606);               /* translate */
            gems_cop_w(0x3E19999A);
            gems_cop_w(0xBE4CCCCD);
            gems_cop_w(0xBE19999A);
            gems_cop_w(0x05000A0A);               /* ang_z */
            gems_cop_w(0xF000);
            gems_cop_w(0x04000808);               /* ang_x */
            gems_cop_w(0x8000);
        } else {
            gems_cop_w(0x02000404);               /* load matrix */
            for (uint32_t i = 0; i < 12; i++)
                gems_cop_w(gems_ld32(0x5010F8 + i * 4));
            uint32_t pos[3];
            gems_ldn(pos, rob + 0x1F4, 3);
            gems_cop_w(0x03000606);
            gems_cop_w(pos[0]);
            gems_cop_w(pos[1]);
            gems_cop_w(pos[2]);
            gems_cop_w(0x04800909);               /* ang_y: facing */
            gems_cop_w((uint32_t)gems_ld16s(rob + 0x26));
            gems_cop_w(0x03000606);
            gems_cop_w(0x3E0F5C29);
            gems_cop_w(0x3E3851EC);
            gems_cop_w(0xBE2E147B);
            frame = gems_ld32(0x500020);
        }
        gems_cop_w(0x04800909);                   /* spin */
        gems_cop_w(frame * 0xD80);
        uint32_t model = (frame & 1) ? gems_ld16(tails + 2) : gems_ld16(tails);
        GEMS_G(1) = 0;
        GEMS_G(0) = model;
        gfn_set_obj();
        gems_cop_w(0x01000202);                   /* pop matrix */
        if (entry) gems_i960_ret();
        return 0;
    }

    uint32_t phase2 = phase + 8;
    if (phase2 >= 0x40) phase2 -= 0x40;
    uint32_t model0 = gems_ld16((uint32_t)swing + phase * 2);
    uint32_t model1 = gems_ld16((uint32_t)swing + phase2 * 2);
    gems_cop_w(0x03000606);
    gems_cop_w(0x3DCCCCCD);
    gems_cop_w(0xBE800000);
    gems_cop_w(0);
    uint32_t turn = (gems_ld32(rob + 0x1A4) & 0x10000) ? 0x8000 : 0xC000;
    gems_cop_w(0x05000A0A);
    gems_cop_w(turn);
    gems_cop_w(0x04800909);
    gems_cop_w(0x1000);
    GEMS_G(1) = 0;
    GEMS_G(0) = model0;
    gfn_set_obj();
    gems_cop_w(0x04800909);
    gems_cop_w(0xFFFFE000);
    GEMS_G(1) = 0;
    GEMS_G(0) = model1;
    gfn_set_obj();
    if (entry) gems_i960_ret();
    return 0;
}
