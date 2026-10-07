/* rob_disp_rdn_loop (trap 0x72 at i960 0x19CD4, Gems 0x80046268): the body
 * of rob_disp_rdn's per-part loop, part r8 of the fighter at g7. It runs on
 * the i960's live registers (r3..r15, g0..g3, g10, g14) and leaves them, and
 * the condition code, as Gems does; R = 0xBB0 resumes the i960 at 0x1A2AC,
 * after the loop body's copy_option_data call.
 *
 * In:  r8 = part index, r11 = object table, g7 = rob, g10 = display list.
 * Out: r3 = the part's object (possibly replaced or zeroed). Gems also
 * loaded r4 from rob+0x7A2, which the i960 does itself on resume.
 *
 * Gems keeps r7..r9 in C locals around the mirror-arm block, where the ROM
 * pushes them on the i960 stack (0x19DA0); the stack words are not written. */
#pragma once

/* Gems' compare of x against c: 4 if x > c, 2 if equal, 1 if less. */
static uint32_t gfn_rob_disp_rdn_loop__ccu(uint32_t x, uint32_t c)
{
    return x > c ? 4u : x == c ? 2u : 1u;
}

static uint32_t gfn_rob_disp_rdn_loop__ccs(int32_t x, int32_t c)
{
    return x > c ? 4u : x == c ? 2u : 1u;
}

/* r15 = word; send it to the COP (the ROM stages every command in r15). */
static void gfn_rob_disp_rdn_loop__w15(uint32_t w)
{
    GEMS_R(15) = w;
    gems_cop_w(w);
}

/* r8 = 5 / 8 with rob+0x1B1 != 13: the arm stretched to the point the
 * fighter reaches for (rob+0x230 / +0x254), like rob_disp_mir_part but on
 * registers and without the Y flip. Returns 0 when neither flag is up. */
static int gfn_rob_disp_rdn_loop__arm(uint32_t rob)
{
    GEMS_R(4) = gems_ld32(rob + 0x2A3C);
    GEMS_AC = (GEMS_R(4) & 0x20) ? 2u : 0u;          /* chkbit 5 */
    uint32_t tip;
    if (GEMS_AC & 2) {
        tip = rob + 0x230;
    } else {
        GEMS_AC = (GEMS_R(4) & 0x100) ? 2u : 0u;     /* chkbit 8 */
        if ((GEMS_AC & 7) == 0)
            return 0;
        tip = rob + 0x254;
    }
    uint32_t save7 = GEMS_R(7), save8 = GEMS_R(8), save9 = GEMS_R(9);
    gems_ldn(&GEMS_R(8), tip, 3);
    gems_ldn(&GEMS_R(4), rob + 0x20C, 3);
    GEMS_R(8)  = gems_f2u(gems_u2f(GEMS_R(8))  - gems_u2f(GEMS_R(4)));
    GEMS_R(9)  = gems_f2u(gems_u2f(GEMS_R(9))  - gems_u2f(GEMS_R(5)));
    GEMS_R(10) = gems_f2u(gems_u2f(GEMS_R(10)) - gems_u2f(GEMS_R(6)));

    gfn_rob_disp_rdn_loop__w15(0x13802727);           /* yaw */
    gems_cop_w(GEMS_R(8));
    gems_cop_w(GEMS_R(10));
    GEMS_R(7) = gems_cop_r();
    gfn_rob_disp_rdn_loop__w15(0x16802D2D);           /* horizontal length */
    gems_cop_w(GEMS_R(8));
    gems_cop_w(GEMS_R(10));
    GEMS_R(10) = gems_cop_r();
    GEMS_R(9) ^= 0x80000000u;
    gfn_rob_disp_rdn_loop__w15(0x13802727);           /* pitch */
    gems_cop_w(GEMS_R(10));
    gems_cop_w(GEMS_R(9));
    GEMS_R(8) = gems_cop_r();
    gfn_rob_disp_rdn_loop__w15(0x16802D2D);           /* length */
    gems_cop_w(GEMS_R(9));
    gems_cop_w(GEMS_R(10));
    GEMS_R(9) = gems_cop_r();
    GEMS_R(10) = 0x3EE66666u;                          /* 0.45 */
    GEMS_R(9) = gems_f2u(gems_u2f(GEMS_R(9)) / gems_u2f(GEMS_R(10)));
    GEMS_R(10) = 0x3F800000u;

    gfn_rob_disp_rdn_loop__w15(0x00800101);           /* push */
    gfn_rob_disp_rdn_loop__w15(0x02000404);           /* load_matrix */
    GEMS_R(15) = rob + 0x2A40;
    for (uint32_t i = 0; i < 12; i++) {
        GEMS_R(14) = gems_ld32(GEMS_R(15) + i * 4);
        gems_cop_w(GEMS_R(14));
    }
    gfn_rob_disp_rdn_loop__w15(0x03000606);           /* translate to base */
    gems_cop_w(GEMS_R(4));
    gems_cop_w(GEMS_R(5));
    gems_cop_w(GEMS_R(6));
    gfn_rob_disp_rdn_loop__w15(0x04800909);
    gems_cop_w(GEMS_R(7));
    gfn_rob_disp_rdn_loop__w15(0x05000A0A);
    gems_cop_w(GEMS_R(8));
    gfn_rob_disp_rdn_loop__w15(0x03800707);           /* scale (len, 1, 1) */
    gems_cop_w(GEMS_R(9));
    gems_cop_w(GEMS_R(10));
    gems_cop_w(GEMS_R(10));

    GEMS_R(4) = gems_ld8(rob + 0x1B0);
    GEMS_R(13) = 0x21;
    GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), GEMS_R(13));
    GEMS_G(1) = 0;
    GEMS_G(0) = (GEMS_AC & 2) ? 0x5CCu : 0x4A5u;
    gfn_set_obj();
    gfn_rob_disp_rdn_loop__w15(0x01000202);           /* pop */
    GEMS_R(9) = save9;
    GEMS_R(8) = save8;
    GEMS_R(7) = save7;
    return 1;
}

/* r8 = 2 on character 13: the blinking eyelid object and its turn. */
static void gfn_rob_disp_rdn_loop__blink(void)
{
    GEMS_R(4) = gems_ld32(0x500020);
    uint32_t t = GEMS_R(4);
    if ((t & 0x80) || (t & 0x40))
        return;
    GEMS_R(5) = (t >> 4) & 3;
    GEMS_R(6) = GEMS_R(4) & 0xF;
    switch (GEMS_R(5)) {
    case 2:
        GEMS_R(6) = 15 - GEMS_R(6);
        /* fall through */
    case 0:
        GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(6), 10);
        if ((GEMS_AC & 3) == 0)
            GEMS_R(6) = 10;
        GEMS_R(3) = gems_ld32(GEMS_R(6) * 4 + 0x1D2F4);
        break;
    case 1:
        GEMS_R(5) = 10;
        GEMS_R(3) = gems_ld32(GEMS_R(5) * 4 + 0x1D2F4);
        GEMS_R(5) = (GEMS_R(4) & 0x100) ? 0xF000u : 0x1000u;
        GEMS_R(6) = GEMS_R(6) * GEMS_R(5);
        gfn_rob_disp_rdn_loop__w15(0x05000A0A);
        gems_cop_w(GEMS_R(6));
        break;
    default:
        break;
    }
}

/* r8 = 2: the two face objects at rob+0x2050 (with their texture points
 * unless rob bit 20 is up or rob+0x1A8 is 0x134). Skipped on screens 14/15
 * for player 0. */
static void gfn_rob_disp_rdn_loop__face(uint32_t rob)
{
    GEMS_R(15) = gems_ld8(0x50002B);
    GEMS_R(14) = 0xE;
    GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(15), GEMS_R(14));
    int screen = (GEMS_AC & 2) != 0;
    if (!screen) {
        GEMS_R(14) = 0xF;
        GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(15), GEMS_R(14));
        screen = (GEMS_AC & 2) != 0;
    }
    if (screen) {
        GEMS_R(14) = gems_ld8(rob + 4);
        GEMS_AC = GEMS_R(14) == 0 ? 2u : 4u;
        if (GEMS_AC & 2)
            return;
    }
    gems_ldn(&GEMS_R(4), rob + 0x2050, 2);
    GEMS_R(15) = gems_ld32(rob);
    if ((GEMS_R(15) & 0x100000) == 0) {
        GEMS_R(13) = 0x134;
        GEMS_R(14) = gems_ld16(rob + 0x1A8);              /* ldos */
        GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(14), GEMS_R(13));
        if ((GEMS_AC & 2) == 0) {
            GEMS_R(12) = gems_ld32(rob + 0x2058);
            GEMS_G(2) = GEMS_R(12);
            GEMS_G(1) = 0;
            GEMS_G(0) = GEMS_R(4);
            gfn_set_obj_tpd(0);
            GEMS_R(12) = gems_ld32(rob + 0x205C);
            GEMS_G(2) = GEMS_R(12);
            GEMS_G(1) = 0;
            GEMS_G(0) = GEMS_R(5);
            gfn_set_obj_tpd(0);
            return;
        }
    }
    GEMS_G(1) = 0;
    GEMS_G(0) = GEMS_R(4);
    gfn_set_obj();
    GEMS_G(1) = 0;
    GEMS_G(0) = GEMS_R(5);
    gfn_set_obj();
}

/* r8 = 2 after the draw: hand the player's slice of the display list over
 * (g10+0x2008 / +0x1008, the per-player links at 0x501804), then the
 * eyelid effects. */
static void gfn_rob_disp_rdn_loop__dl_link(uint32_t rob)
{
    uint32_t g10 = GEMS_G(10);
    GEMS_R(15) = gems_ld32(g10 + 0x2008);
    gems_st32(0x50EFFC, GEMS_G(14));
    GEMS_G(14) = 0;
    gems_st32(g10, GEMS_G(14));
    GEMS_G(14) = gems_ld32(0x50EFFC);
    gems_st32(0x501800, GEMS_R(15));

    GEMS_R(15) = (uint32_t)gems_ld8(rob + 4) * 8 + 0x501804;
    GEMS_R(14) = gems_ld32(GEMS_R(15) + 4);
    GEMS_R(13) = gems_ld32(g10 + 0x2008);
    GEMS_AC = GEMS_R(14) == 0 ? 2u : 4u;
    if ((GEMS_AC & 5) == 0) {
        gems_st32(GEMS_R(15), GEMS_R(13));
    } else {
        GEMS_R(13) |= 0x80000000u;
        gems_st32(GEMS_R(14) + 0x900000, GEMS_R(13));
    }
    GEMS_R(15) = gems_ld32(0x501800);
    gems_st32(g10 + 0x1008, GEMS_R(15));

    GEMS_R(15) = (uint32_t)gems_ld8(rob + 4) * 8 + 0x501804;
    GEMS_R(14) = gems_ld32(GEMS_R(15));
    GEMS_R(13) = gems_ld32(g10 + 0x2008);
    gems_st32(GEMS_R(15) + 4, GEMS_R(13));
    GEMS_R(14) |= 0x80000000u;
    gems_st32(g10, GEMS_R(14));
    gfn_efc_asease_cont(0);
    gfn_efc_mabuta_cont(0);
}

static uint32_t gfn_rob_disp_rdn_loop(int entry)
{
    (void)entry;
    uint32_t rob = GEMS_G(7);

    gfn_rob_disp_rdn_loop__w15(0x00800101);           /* push */
    GEMS_R(4) = rob + 0x2758;
    GEMS_R(5) = gems_ld32(GEMS_R(4) + 0x68);
    GEMS_R(4) = 0x3F800000u;
    gfn_rob_disp_rdn_loop__w15(0x03800707);           /* scale (1, s, 1) */
    gems_cop_w(GEMS_R(4));
    gems_cop_w(GEMS_R(5));
    gems_cop_w(GEMS_R(4));
    gfn_rob_disp_rdn_loop__w15(0x1B803737);           /* unit matrix */
    GEMS_R(15) = gems_ld8(rob + 4);
    gems_cop_w(GEMS_R(15));
    GEMS_R(15) = GEMS_R(8) * 12;
    gems_cop_w(GEMS_R(15));
    GEMS_R(3) = gems_ld32(GEMS_R(11) + GEMS_R(8) * 4);
    GEMS_G(0) = 3;
    gfn_set_mmode(0);

    GEMS_R(4) = gems_ld32(rob + 0x2068);
    GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 1);
    if ((GEMS_AC & 5) == 0) {
        /* Spin-attack ball: only part 0 draws, as the spin effect. */
        GEMS_AC = GEMS_R(8) == 0 ? 2u : 4u;
        if ((GEMS_AC & 5) == 0) {
            GEMS_R(4) = gems_ld8(rob + 0x1B1) % 26;
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 7);
            GEMS_G(3) = rob + 0x2088;
            gems_ldn(&GEMS_G(0), GEMS_G(3) + 0x24, 3);
            if ((GEMS_AC & 2) == 0)
                gfn_spin_attack_cnt_nml_dsp(0);
            else
                gfn_spin_attack_cnt_esp_dsp(0);
        }
    } else {
        int32_t part = (int32_t)GEMS_R(8);
        int arm_part;
        GEMS_AC = gfn_rob_disp_rdn_loop__ccs(part, 5);
        arm_part = (GEMS_AC & 2) != 0;
        if (!arm_part) {
            GEMS_AC = gfn_rob_disp_rdn_loop__ccs(part, 8);
            arm_part = (GEMS_AC & 5) == 0;
        }
        if (arm_part) {
            GEMS_R(14) = gems_ld8(rob + 0x1B1);
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(14), 13);
            if ((GEMS_AC & 2) == 0) {
                gfn_rob_disp_rdn_loop__arm(rob);
            } else {
                /* Character 13 turns these parts with the frame counter. */
                GEMS_R(4) = gems_ld32(0x500020);
                GEMS_R(15) = gems_ld32(rob + 0x804);
                if ((GEMS_R(15) & 0x100) == 0) {
                    GEMS_R(5) = 0x3F;
                    GEMS_R(4) = GEMS_R(5) & GEMS_R(4);
                    GEMS_R(5) = 0x400;
                } else {
                    GEMS_R(4) = GEMS_R(4) & 0xF;
                    GEMS_R(5) = 0x1000;
                }
                GEMS_R(4) = GEMS_R(4) * GEMS_R(5);
                gfn_rob_disp_rdn_loop__w15(0x04000808);
                gems_cop_w(GEMS_R(4));
            }
        }

        /* Part 1 (chest) extras. */
        GEMS_AC = gfn_rob_disp_rdn_loop__ccs((int32_t)GEMS_R(8), 1);
        if ((GEMS_AC & 5) == 0) {
            GEMS_R(4) = gems_ld8(rob + 0x1B1);
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 12);
            if ((GEMS_AC & 5) == 0) {
                GEMS_G(1) = 0;
                GEMS_G(0) = 0xD1A;
                gfn_set_obj();
                GEMS_G(1) = 0;
                GEMS_G(0) = 0xB20;
                gfn_set_obj();
            }
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 4);
            if ((GEMS_AC & 5) == 0) {
                gfn_efc_fang_mune_init(0);
                gfn_efc_fang_mune_change(0);
                GEMS_R(15) = gems_ld32(rob + 0x70C);
                if (GEMS_R(15) & 0x800000)
                    GEMS_R(3) = 0;
            }
        }

        /* Part 2 (head) extras. */
        GEMS_AC = gfn_rob_disp_rdn_loop__ccs((int32_t)GEMS_R(8), 2);
        if ((GEMS_AC & 5) == 0) {
            GEMS_R(14) = gems_ld8(rob + 0x1B1);
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(14), 13);
            if ((GEMS_AC & 5) == 0)
                gfn_rob_disp_rdn_loop__blink();
            gfn_rob_disp_rdn_loop__face(rob);
        }

        /* The part itself; on screens 14/15 only player 1's. */
        GEMS_R(15) = gems_ld8(0x50002B);
        GEMS_R(14) = 0xE;
        GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(15), GEMS_R(14));
        int screen = (GEMS_AC & 2) != 0;
        if (!screen) {
            GEMS_R(14) = 0xF;
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(15), GEMS_R(14));
            screen = (GEMS_AC & 2) != 0;
        }
        int draw = 1;
        if (screen) {
            GEMS_R(14) = gems_ld8(rob + 4);
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(14), 1);
            draw = (GEMS_AC & 2) != 0;
        }
        if (draw) {
            GEMS_G(1) = gems_ld8(rob + 4);
            GEMS_G(0) = GEMS_R(3);
            gfn_set_obj();
        }
    }

    GEMS_G(0) = 1;
    gfn_set_mmode(0);

    int32_t part = (int32_t)GEMS_R(8);
    GEMS_R(4) = gems_ld32(0x1A184u + GEMS_R(8) * 4u);  /* the jump table entry the ROM bx's through */
    if (part == 1) {
        gfn_efc_eggrob_mune_chg(0);
        GEMS_R(4) = gems_ld8(rob + 0x1B1);
        GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 4);
        if ((GEMS_AC & 5) == 0)
            gfn_efc_fang_gun_disp(0);                   /* Gems inlines it */
        GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 3);
        if ((GEMS_AC & 5) == 0)
            gfn_efc_metalsonic_disp(0);
    } else if (part == 9) {
        GEMS_R(15) = gems_ld32(0x5004C8);
        if ((GEMS_R(15) & 0x20) == 0) {
            GEMS_R(4) = gems_ld8(rob + 0x1B0);
            GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 1);
            if (GEMS_AC & 2) {
                gfn_tails_tail_disp(0, 0x1AF34, 0x1AE34);
            } else {
                GEMS_AC = gfn_rob_disp_rdn_loop__ccu(GEMS_R(4), 0x1B);
                if (GEMS_AC & 2)
                    gfn_tails_tail_disp(0, 0x1AF38, 0x1AEB4);
            }
        }
    } else if (part == 2 || part < 0 || part >= 16) {
        /* The ROM's jump table (0x1A184) has 16 entries and only 2 goes
         * here; Gems also sends negative and >= 16 parts this way. */
        gfn_rob_disp_rdn_loop__dl_link(rob);
    }

    gfn_copy_option_data(0);
    return 0xBB0;    /* the i960's ldos 0x7A2(g7), r4 at 0x1A2AC runs on resume */
}
