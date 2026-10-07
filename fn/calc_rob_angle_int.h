/* calc_rob_angle_int (i960 0x2EF38; Gems 80054050): a fighter's (g7) motion
 * set-up.
 *   - When byte g7+0x84C differs from byte g7+0x85B: picks the model table
 *     (0xC5268[char], +0x40 from set 2 on; from set 3 the table's third word,
 *     with Bean/Metal (7 / 0x21) flagging g7+0x2A3C 0x100 or 0x20 by bit 6 of
 *     *g7, and +0x40 under bit 6), copies its 16 models to g7+0x40, copies the
 *     motion's 45 words (0xC2068[set][char]) to g7+0x8C (mirrored under bit 6:
 *     through the signed index table at 0x316A4, a negative index flipping the
 *     word's sign bit), and calls send_init_ram_coli_data.
 *   - When g7+0x1A8 names a pose (n != 0): unpacks the pose record at
 *     *(0x6400004 + n*4) into the per-unit kinds at g6+0x78C (g6 = *(g7+0xBD8)),
 *     sets g6+0x780/0x784/0x788 to its data/kind/count pointers, then walks the
 *     kinds: kind 0 loads g7+0x80, kinds 1 and 2 write truncated angles to the
 *     unit's record (table 0x316E4[row-12]) and set its bit in g7+0xBDC.
 * Angles are converted with cvtri (rounded by the AC mode; out of range or
 * NaN gives 0x80000000), as the i960 does. Leaves g2, g5 and g6 as the i960
 * does. */
#pragma once

static uint32_t gfn_calc_rob_angle_int(int entry)
{
    uint32_t set = gems_ld8(GEMS_G(7) + 0x84Cu);
    if (set != gems_ld8(GEMS_G(7) + 0x85Bu)) {
        uint32_t chr = gems_ld8(GEMS_G(7) + 0x1B0u);
        uint32_t motion = gems_ld32(gems_ld32(0xC2068u + set * 4u) + chr * 4u);
        uint32_t tbl = gems_ld32(0xC5268u + chr * 4u);
        gems_st32(GEMS_G(7) + 0x2A3Cu, 0);
        uint32_t flags = gems_ld32(GEMS_G(7));
        uint32_t src = gems_ld32(tbl);
        if (set > 1u) {
            src += 0x40u;
            if (set > 2u) {
                src = gems_ld32(tbl + 8u);
                uint32_t c = gems_ld8(GEMS_G(7) + 0x1B0u);
                if (c == 7u || c == 0x21u) {
                    uint32_t f = gems_ld32(GEMS_G(7) + 0x2A3Cu);
                    f |= (flags & 0x40u) ? 0x100u : 0x20u;
                    gems_st32(GEMS_G(7) + 0x2A3Cu, f);
                }
                if (flags & 0x40u)
                    src += 0x40u;
            }
        }
        uint32_t dst = GEMS_G(7) + 0x40u;
        for (uint32_t i = 0; i < 16u; i++)
            gems_st32(dst + i * 4u, gems_ld32(src + i * 4u));

        dst = GEMS_G(7) + 0x8Cu;
        if ((flags & 0x40u) == 0) {
            for (uint32_t i = 0; i < 45u; i++)
                gems_st32(dst + i * 4u, gems_ld32(motion + i * 4u));
        } else {
            for (uint32_t i = 0; i < 45u; i++) {
                uint32_t idx = (uint32_t)(int32_t)(int8_t)gems_ld8(0x316A4u + i);
                uint32_t v = gems_ld32(motion + i * 4u);
                if (idx & 0x80000000u) {
                    idx = 0u - idx;
                    v ^= 0x80000000u;
                }
                gems_st32(dst + idx * 4u, v);
            }
        }
        gfn_send_init_ram_coli_data(0);
    }

    uint32_t pose = gems_ld16(GEMS_G(7) + 0x1A8u);
    if (pose != 0) {
        uint32_t p = gems_ld32(0x6400004u + pose * 4u) + 2u;   /* kind bytes */
        uint32_t q = p + 20u;                                   /* counts */
        uint32_t sum = 0, cnt = 0;
        GEMS_G(6) = gems_ld32(GEMS_G(7) + 0xBD8u);
        GEMS_G(5) = GEMS_G(6) + 0x78Cu;
        for (uint32_t row = 0; row < 20u; row++, p++) {
            for (uint32_t shift = 0; shift < 6u; shift += 2u) {
                uint32_t b = gems_ld8(p);
                uint32_t v = (b >> 6) & 3u;
                v = v ? v - 1u : ((b >> shift) & 3u) + 3u;
                gems_st8(GEMS_G(5), v);
                GEMS_G(5)++;
                if (v > 4u) {
                    sum += gems_ld8(q);
                    q++;
                    cnt++;
                }
            }
        }
        gems_st32(GEMS_G(6) + 0x784u, p);
        q += (4u - ((cnt + 22u) & 3u)) & 3u;
        gems_st32(GEMS_G(6) + 0x788u, q);
        uint32_t data = q + sum * 4u;
        gems_st32(GEMS_G(6) + 0x780u, data);

        GEMS_G(2) = p;
        GEMS_G(5) = GEMS_G(6) + 0x78Cu;
        gems_st8(GEMS_G(7) + 0xBDCu, 0);
        for (uint32_t row = 0; row < 20u; row++) {
            for (uint32_t j = 0; j < 3u; j++) {
                uint32_t v = gems_ld8(GEMS_G(5));
                if (v > 5u) {
                    data += gems_ld8(GEMS_G(2)) * 12u;
                    GEMS_G(2)++;
                } else if (v == 5u) {
                    data += gems_ld8(GEMS_G(2)) * 4u;
                    GEMS_G(2)++;
                } else if (v == 4u) {
                    (void)gems_ld32(data);              /* read and dropped */
                    data += 4u;
                } else if (v == 3u) {
                    /* nothing */
                } else {
                    if (v == 0) {
                        uint32_t t[3];
                        gems_ldn(t, data, 3);
                        gems_stn(GEMS_G(7) + 0x80u, t, 3);
                        data += 12u;
                    } else {
                        uint32_t s = row - 12u;
                        uint32_t bits = gems_ld8(GEMS_G(7) + 0xBDCu);
                        bits |= 1u << (s & 31u);                    /* setbit: mod 32 */
                        gems_st8(GEMS_G(7) + 0xBDCu, bits & 0xFFu);
                        uint32_t rec = gems_ld32(0x316E4u + s * 4u);
                        rec += GEMS_G(7);
                        if (v == 1u) {
                            uint32_t t[3];
                            gems_ldn(t, data, 3);
                            for (uint32_t k = 0; k < 3u; k++)
                                gems_st16(rec + k * 2u,
                                          gems_cvtri(t[k]) & 0xFFFFu);
                            data += 12u;
                        } else {
                            /* the first word, then three skipped one word on:
                             * the ROM's own layout */
                            uint32_t w = gems_ld32(data);
                            data += 4u;
                            gems_st16(rec,
                                      gems_cvtri(w) & 0xFFFFu);
                            for (uint32_t k = 1; k < 4u; k++)
                                gems_st16(rec + k * 2u,
                                          gems_cvtri(gems_ld32(data + k * 4u)) & 0xFFFFu);
                            data += 16u;
                        }
                    }
                    GEMS_G(5) += 3u;
                    break;
                }
                GEMS_G(5)++;
            }
        }
    }

    if (entry) gems_i960_ret();
    return 0;
}
