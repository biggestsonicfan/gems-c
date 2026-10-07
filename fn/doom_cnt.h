/* doom_cnt (i960 0x28610; Gems 80045A2C): the dome's sixteen panels. Not on
 * stages 3 and 10, nor on stage 9 while bit 4 of 0x500498 is set. Pushes the
 * matrix, reads the eye (0x58) and the world position (0x0F), sets up the
 * dome's frame (0x43 7, base matrix, turns by the negated angles at
 * 0x50A020..24, and unless on stage 4 the negated translation at 0x50A014),
 * then turns by the spin at 0x500464 (which advances by 2 when bit 27 of the
 * stage record at 0x8F3D0 + stage * 256 is set). Each panel turns a further
 * 0x1000; a panel is drawn (set_obj, model from the table at
 * 0x8F490 + stage * 256) when bit 27 of the word at g13 is set or when its
 * facing ratio is not > g1 (0.866; cmpr + bl, so an unordered ratio draws).
 * Leaves g1 = 0x3F5DB3D7 unless a panel was drawn. */
#pragma once

static uint32_t gfn_doom_cnt(int entry)
{
    uint32_t stage = gems_ld8(0x500064u);
    if (stage == 10u || stage == 3u ||
        (stage == 9u && (gems_ld32(0x500498u) & 0x10u) != 0)) {
        if (entry) gems_i960_ret();
        return 0;
    }

    gems_cop_w(0x00800101u);                    /* push matrix */
    gems_cop_w(0x2C005858u);                    /* eye */
    uint32_t eye_x = gems_cop_r();
    (void)gems_cop_r();
    uint32_t eye_z = gems_cop_r();
    GEMS_G(1) = 0x3F5DB3D7u;                    /* 0.866f */
    gems_cop_w(0x07800F0Fu);                    /* get_point */
    float pos_x = gems_cop_rf();
    (void)gems_cop_r();
    float pos_z = gems_cop_rf();

    gems_cop_w(0x21804343u);
    gems_cop_w(7u);
    gems_cop_w(0x01800303u);                    /* base matrix */
    gems_cop_w(0x05000A0Au);                    /* ang_z */
    gems_cop_w((uint32_t)-(int32_t)gems_ld16s(0x50A024u));
    gems_cop_w(0x04000808u);                    /* ang_x */
    gems_cop_w((uint32_t)-(int32_t)gems_ld16s(0x50A020u));
    gems_cop_w(0x04800909u);                    /* ang_y */
    gems_cop_w((uint32_t)-(int32_t)gems_ld16s(0x50A022u));
    if (stage != 4u) {
        gems_cop_w(0x03000606u);                /* trans, negated */
        gems_cop_w(gems_ld32(0x50A014u) ^ 0x80000000u);
        gems_cop_w(gems_ld32(0x50A018u) ^ 0x80000000u);
        gems_cop_w(gems_ld32(0x50A01Cu) ^ 0x80000000u);
    }

    uint32_t spin = gems_ld16(0x500464u);
    gems_cop_w(0x04800909u);                    /* ang_y */
    gems_cop_w(spin);
    uint32_t rec = gems_ld8(0x500064u) * 0x100u;
    if (gems_ld32(rec + 0x8F3D0u) & 0x08000000u)
        spin += 2u;
    gems_st16(0x500464u, spin & 0xFFFFu);
    uint32_t models = rec + 0x8F490u;

    const uint32_t probe_xy = 0xC21C126Fu;     /* -39.018f */
    const uint32_t probe_z = 0x43442831u;      /* 196.157f */
    for (uint32_t panel = 0; panel < 16u; panel++) {
        gems_cop_w(0x04800909u);                /* ang_y */
        gems_cop_w(0x1000u);
        int draw = (gems_ld32(GEMS_G(13)) & 0x08000000u) != 0;
        if (!draw) {
            gems_cop_w(0x14802929u);            /* point_trans */
            gems_cop_w(probe_xy);
            gems_cop_w(probe_xy);
            gems_cop_w(probe_z);
            float px = gems_cop_rf();
            (void)gems_cop_r();
            float pz = gems_cop_rf();
            float dx = pos_x - px;
            float dz = pz - pos_z;
            gems_cop_w(0x16802D2Du);            /* get_2d_len */
            gems_cop_wf(dx);
            gems_cop_wf(dz);
            float len = gems_cop_rf();
            gems_cop_w(0x2C805959u);
            gems_cop_w(eye_x);
            gems_cop_wf(dx);
            gems_cop_w(eye_z);
            gems_cop_wf(dz);
            float ratio = gems_cop_rf() / len;
            draw = !(gems_u2f(GEMS_G(1)) < ratio);   /* cmpr g1, r4; bl skips */
        }
        if (draw) {
            gems_cop_w(0x00800101u);            /* push matrix */
            gems_cop_w(0x23004646u);
            gems_cop_w(7u);
            if (gems_ld8(0x500064u) != 2u) {
                gems_cop_w(0x03800707u);        /* scale 1.6 */
                gems_cop_w(0x3FCCCCCDu);
                gems_cop_w(0x3FCCCCCDu);
                gems_cop_w(0x3FCCCCCDu);
            }
            uint16_t model = (uint16_t)gems_ld16(models + (panel & 3u) * 2u);
            GEMS_G(1) = 0;
            GEMS_G(0) = model;
            gfn_set_obj();
            gems_cop_w(0x01000202u);            /* pop matrix */
        }
    }
    gems_cop_w(0x01000202u);                    /* pop matrix */

    if (entry) gems_i960_ret();
    return 0;
}
