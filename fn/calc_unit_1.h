/* calc_unit_1 (i960 0x30FE8; Gems 8003A4A8, called natively, not a trap):
 * one part of the fighter's unit-matrix chain (g1 = part index, advanced on
 * return with g0/g3/g4/g5/g6).
 *  - Part 2 is first tilted by rob+0xBE6 (ang_x); then the part's offset (g6)
 *    is translated, COP 0x3F is sent the joint's three angles (g5, reply
 *    dropped), and the motion's own y/x/z angles (g3) are applied.
 *  - A part whose bit in rob+0xBDC is set takes its look angles (g0+4 /
 *    g0+2) as stored.  Otherwise they come from the point g4 seen through
 *    the current matrix (COP 0x6A, then atan2 0x27 and the length 0x2D):
 *    part 1 first eases rob+0xBE6 (the waist bend towards the enemy),
 *    part 2 (the head) turns towards the target at most 0x800/0x400 a frame
 *    (0x2000 when rob+0x7D3 is clear) and within +-0x2000.
 * The bit test is PPC slw (a shift of 32..63 tests nothing), where the i960's
 * bbc takes the index mod 32; the index is a part number, below 8. */
#pragma once

static uint32_t gfn_calc_unit_1(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t r[3];

    if (GEMS_G(1) == 2) {
        gems_cop_w(0x04000808);                            /* ang_x */
        gems_cop_w((uint32_t)gems_ld16s(rob + 0xBE6));
    }
    gems_cop_w(0x03000606);                                /* trans */
    gems_ldn(r, GEMS_G(6), 3);
    gems_cop_wn(r, 3);
    gems_cop_w(0x1F803F3F);
    r[0] = (uint32_t)gems_ld16s(GEMS_G(5) + 4);
    r[1] = (uint32_t)gems_ld16s(GEMS_G(5) + 2);
    r[2] = (uint32_t)gems_ld16s(GEMS_G(5));
    gems_cop_wn(r, 3);
    (void)gems_cop_r();
    GEMS_G(5) += 6;
    gems_cop_w(0x04800909);                                /* ang_y */
    gems_cop_w((uint32_t)gems_ld16s(GEMS_G(3) + 2));
    gems_cop_w(0x04000808);                                /* ang_x */
    gems_cop_w((uint32_t)gems_ld16s(GEMS_G(3)));
    gems_cop_w(0x05000A0A);                                /* ang_z */
    gems_cop_w((uint32_t)gems_ld16s(GEMS_G(3) + 4));
    GEMS_G(3) += 6;

    uint32_t mask = gems_ld8(rob + 0xBDC);
    uint32_t sh   = GEMS_G(1) & 63;
    uint32_t angz, angy;

    if (sh < 32 && (mask & (1u << sh))) {
        angz = gems_ld16(GEMS_G(0) + 4);
        angy = gems_ld16(GEMS_G(0) + 2);
    } else {
        if (GEMS_G(1) == 1) {
            uint32_t flags = gems_ld32(rob + 0x1A4);
            if ((flags & 0x16000) || !(flags & 0x20C)) {
                gems_st16(rob + 0xBE6, 0);
            } else {
                /* ease the waist towards the enemy's direction */
                gems_cop_w(0x35006A6A);
                gems_ldn(r, rob + 0xB0C, 3);
                gems_cop_wn(r, 3);
                gems_cop_rn(r, 3);
                gems_cop_w(0x13802727);                    /* atan2 */
                gems_cop_w(r[0]);
                gems_cop_w(r[2]);
                int32_t want = (int16_t)gems_cop_r();
                if (gems_ld32(rob) & 0x40)
                    want = -want;
                bool store = true;
                if (want < 0)
                    want = 0;
                else if (want <= 0x2000)
                    want >>= 1;
                else if (want <= 0x6000)
                    want = 0x1000;
                else
                    store = false;
                if (store) {
                    int32_t cur = gems_ld16s(rob + 0xBE6);
                    if (gems_ld32(rob) & 0x40)
                        cur = -cur;
                    int32_t d = want - cur;
                    if (d > 0x100) {
                        want = cur + 0x100;
                        if (want > 0x1000)
                            want = 0x1000;
                    } else if (d < -0x100) {
                        want = cur - 0x100;
                        if (want < 0)
                            want = 0;
                    }
                    if (gems_ld32(rob) & 0x40)
                        want = -want;
                    gems_st16(rob + 0xBE6, (uint16_t)want);
                }
            }
        }

        gems_cop_w(0x35006A6A);
        gems_ldn(r, GEMS_G(4), 3);
        gems_cop_wn(r, 3);
        gems_cop_rn(r, 3);

        bool head = false;
        if (GEMS_G(1) == 2) {
            uint32_t f = gems_ld32(rob);
            if (f & 0x200000)
                gems_st32(rob, f & ~0x200000u);
            else
                head = true;
        }

        if (head) {
            /* the head: turn towards the point, a step at a time */
            bool turn = true;
            if ((int32_t)r[1] < 0) {
                r[1] = 0;
                float x = gems_u2f(r[0]);
                if ((int32_t)r[0] < 0) {
                    if (x >= -0.5f && gems_ld16(GEMS_G(0) + 4) == 0)
                        turn = false;
                } else {
                    if (!(0.5f < x) && gems_ld16(GEMS_G(0) + 4) == 0x8000)
                        turn = false;
                }
            }
            if (turn) {
                gems_cop_w(0x13802727);                    /* atan2 */
                gems_cop_w(r[0]);
                gems_cop_w(r[1]);
                uint32_t neg = 0u - gems_cop_r();
                int32_t cur = gems_ld16s(GEMS_G(0) + 4);
                int32_t lim = gems_ld8(rob + 0x7D3) ? 0x800 : 0x2000;
                int32_t a   = (int32_t)(neg - (uint32_t)cur);
                if (a >= lim)
                    a = lim;
                else if (a < -lim)
                    a = -lim;
                gems_st16(GEMS_G(0) + 4, (uint16_t)((uint32_t)a + (uint32_t)cur));
            }
            gems_cop_w(0x16802D2D);                        /* length */
            gems_cop_w(r[0]);
            gems_cop_w(r[1]);
            uint32_t len = gems_cop_r();
            gems_cop_w(0x13802727);                        /* atan2 */
            gems_cop_w(len);
            gems_cop_w(r[2]);
            int32_t b = (int16_t)gems_cop_r();
            GEMS_G(2) = (uint32_t)gems_ld16s(GEMS_G(0) + 2);
            int32_t cur = (int32_t)GEMS_G(2);
            int32_t lim = gems_ld8(rob + 0x7D3) ? 0x400 : 0x2000;
            b -= cur;
            if (b >= lim)
                b = lim;
            else if (b < -lim)
                b = -lim;
            b += cur;
            if (b >= 0x2000)
                b = 0x2000;
            else if (b < -0x2000)
                b = -0x2000;
            gems_st16(GEMS_G(0) + 2, (uint16_t)b);
            goto next;
        }

        gems_cop_w(0x13802727);                            /* atan2 */
        gems_cop_w(r[0]);
        gems_cop_w(r[1]);
        angz = 0u - (uint32_t)(int16_t)gems_cop_r();
        gems_st16(GEMS_G(0) + 4, (uint16_t)angz);
        gems_cop_w(0x16802D2D);                            /* length */
        gems_cop_w(r[0]);
        gems_cop_w(r[1]);
        uint32_t len = gems_cop_r();
        gems_cop_w(0x13802727);                            /* atan2 */
        gems_cop_w(len);
        gems_cop_w(r[2]);
        angy = (uint32_t)(int16_t)gems_cop_r();
        gems_st16(GEMS_G(0) + 2, (uint16_t)angy);
    }
    gems_cop_w(0x05000A0A);                                /* ang_z */
    gems_cop_w(angz);
    gems_cop_w(0x04800909);                                /* ang_y */
    gems_cop_w(angy);

next:
    GEMS_G(6) += 12;
    GEMS_G(4) += 12;
    GEMS_G(0) += 6;
    GEMS_G(1) += 1;
    if (entry) gems_i960_ret();
    return 0;
}
