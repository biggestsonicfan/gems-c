/* ring_tobitiri (i960 0x78DF8, trapped at 0x78E58; Gems 80051ED8): move and
 * draw the scattered rings.  The rings are 44-byte records at 0x574010 in a
 * list (head 0x574000, tail 0x574004, 0x63 = empty, busy mask 0x574008):
 *   +0 next, +4 age, +8 x/y/z, +0x14 x/z speed, +0x1C fall table, +0x20 age
 *   from which it blinks, +0x24 lifetime, +0x28 model kind (0 = a ring).
 * Unless the game is paused (0x508000 bit 5) a ring ages, dies at its
 * lifetime (unlinked; the last one empties the list), falls by its table,
 * bounces off the +-7.5 walls and loses speed when it lands (* 0.7); then it
 * is drawn, with its reflection when 0x500064 == 2.
 * The trap enters with r11 = r12 = the list head (0x78E40-0x78E48).
 * Two places follow Gems where the i960 differs: the busy bit is cleared with
 * PPC slw (an index of 32..63 clears nothing; the i960's clrbit takes it mod
 * 32), and a NaN position is left alone at the walls (the i960's cmpr with a
 * NaN does not branch, so it clamps). */
#pragma once

static uint32_t gfn_ring_tobitiri(int entry)
{
    uint32_t r4 = 0, r5 = 0, r6 = 0, r7, r8 = 0, r9 = 0, r10, r14;
    uint32_t r11 = GEMS_R(11);
    uint32_t r12 = GEMS_R(12);
    uint32_t r13 = gems_ld32(0x574004);

    for (;;) {
        r7 = 0x574010 + 44 * r12;
        bool skip_move = false;
        if (!(gems_ld32(0x508000) & 0x20)) {
            r10 = gems_ld32(r7 + 4) + 1;
            gems_st32(r7 + 4, r10);
            r9 = gems_ld32(r7 + 0x24);
            if (!(r9 > r10)) {
                /* the ring's life is over: unlink it */
                uint32_t sh = r12 & 63;
                uint32_t bit = sh < 32 ? (1u << sh) : 0;
                gems_st32(0x574008, gems_ld32(0x574008) & ~bit);
                if (r11 == r12) {
                    if (r12 == r13) {
                        /* the last ring: empty the list (0x78750) */
                        static const uint32_t zero2[2] = { 0, 0 };
                        gems_stn(0x574008, zero2, 2);
                        gems_st32(0x574000, 0x63);
                        gems_st32(0x574B18, 0);
                        gems_st32(0x574B18 + 4, 0);
                        break;
                    }
                    r12 = gems_ld32(r7);
                    r11 = r12;
                    gems_st32(0x574000, r11);
                    continue;
                }
                if (r12 == r13) {
                    r13 = r11;
                    gems_st32(0x574004, r13);
                    break;
                }
                r4 = gems_ld32(r7);
                gems_st32(0x574010 + 44 * r11, r4);
                r12 = r4;
            }

            /* 0x78F00: move it */
            uint32_t t[3];
            gems_ldn(t, r7 + 8, 3);
            r4 = t[0]; r5 = t[1]; r6 = t[2];
            if (r5 & 0x80000000u) {
                r5 = 0;
            } else {
                r8 = gems_ld32(r7 + 0x14);
                r9 = gems_ld32(r7 + 0x18);
                r4 = gems_f2u(gems_u2f(r4) + gems_u2f(r8));
                r6 = gems_f2u(gems_u2f(r6) + gems_u2f(r9));
                r14 = 0x40F00000;                              /* 7.5 */
                if (gems_u2f(r14) <= gems_u2f(r4 & 0x7FFFFFFFu)) {
                    r8 ^= 0x80000000u;
                    r4 = (r4 & 0x80000000u) ? (r14 | 0x80000000u) : (r14 & 0x7FFFFFFFu);
                    gems_st32(r7 + 0x14, r8);
                }
                if (gems_u2f(r14) <= gems_u2f(r6 & 0x7FFFFFFFu)) {
                    r9 ^= 0x80000000u;
                    r6 = (r6 & 0x80000000u) ? (r14 | 0x80000000u) : (r14 & 0x7FFFFFFFu);
                    gems_st32(r7 + 0x18, r9);
                }
                r10 = gems_ld32(r7 + 4);
                r14 = gems_ld32(r7 + 0x1C);
                r5 = gems_ld32(r14 + r10 * 4);
                t[0] = r4; t[1] = r5; t[2] = r6;
                if (r5 & 0x80000000u) {
                    gems_stn(r7 + 8, t, 3);
                    r5 = 0;
                } else {
                    if (r5 == 0) {
                        /* on the floor: slow down */
                        float k = gems_u2f(0x3F333333);
                        uint32_t s[2];
                        r8 = gems_f2u(k * gems_u2f(r8));
                        r9 = gems_f2u(k * gems_u2f(r9));
                        s[0] = r8; s[1] = r9;
                        gems_stn(r7 + 0x14, s, 2);
                    }
                    gems_stn(r7 + 8, t, 3);
                }
            }
            skip_move = true;
        }
        if (!skip_move) {
            /* paused: only the position */
            uint32_t t[3];
            gems_ldn(t, r7 + 8, 3);
            r4 = t[0]; r5 = t[1]; r6 = t[2];
            if (r5 & 0x80000000u)
                r5 = 0;
        }

        /* 0x78FB0: draw it, blinking at the end of its life */
        r9  = gems_ld32(r7 + 4);
        r10 = gems_ld32(r7 + 0x20);
        if (r10 > r9 || !(r9 & 2)) {
            r14 = gems_ld32(r7 + 0x28);
            if (r14 == 0) {
                r9 &= 15;
                r8 = gems_ld32(0xAE314 + r9 * 4);
                gems_cop_w(0x00800101);                        /* push */
                gems_cop_w(0x03000606);                        /* trans */
                gems_cop_w(r4);
                gems_cop_w(r5);
                gems_cop_w(r6);
                GEMS_G(1) = 0;
                GEMS_G(0) = r8;
                gfn_set_obj();
                if (gems_ld8(0x500064) == 2) {
                    r5 = gems_f2u(gems_u2f(r5) + gems_u2f(r5)) | 0x80000000u;
                    r4 = 0;
                    r8 = gems_ld32(0xAE368 + r9 * 4);
                    gems_cop_w(0x03000606);                    /* trans */
                    gems_cop_w(r4);
                    gems_cop_w(r5);
                    gems_cop_w(r4);
                    GEMS_G(1) = 0;
                    GEMS_G(0) = r8;
                    gfn_set_obj();
                }
                gems_cop_w(0x01000202);                        /* pop */
            } else {
                r8 = gems_ld32(r7 + 0x28) - 1;
                r8 = gems_ld32(0xAE354 + r8 * 4);
                r9 = (r9 & 15) << 12;
                gems_cop_w(0x00800101);                        /* push */
                gems_cop_w(0x03000606);                        /* trans */
                gems_cop_w(r4);
                gems_cop_w(r5);
                gems_cop_w(r6);
                gems_cop_w(0x04000808);                        /* ang_x */
                gems_cop_w(r9);
                GEMS_G(1) = 0;
                GEMS_G(0) = r8;
                gfn_set_obj();
                gems_cop_w(0x01000202);                        /* pop */
                if (gems_ld8(0x500064) == 2) {
                    gems_cop_w(0x00800101);                    /* push */
                    gems_cop_w(0x03800707);                    /* scale */
                    gems_cop_w(0x3F800000);
                    gems_cop_w(0xBF800000);
                    gems_cop_w(0x3F800000);
                    gems_cop_w(0x03000606);                    /* trans */
                    gems_cop_w(r4);
                    gems_cop_w(r5);
                    gems_cop_w(r6);
                    gems_cop_w(0x04000808);                    /* ang_x */
                    gems_cop_w(r9);
                    GEMS_G(1) = 0;
                    GEMS_G(0) = r8;
                    gfn_set_obj();
                    gems_cop_w(0x01000202);                    /* pop */
                }
            }
        }

        /* 0x79158: the next ring */
        r11 = r12;
        r12 = gems_ld32(r7);
        if (r11 == r13)
            break;
    }

    if (entry) gems_i960_ret();
    return 0;
}
