/* unit_smooth_cancel_rear (i960 0x2FC40; Gems 8003F7CC): copy the parts the
 * smoothing cancels from one unit-matrix set to another. Bits 13-16 of mask
 * name up to four groups, done from the highest; each group is a 4-byte
 * record of three byte offsets (table 0x2FD64, or 0x2FD74 when bit 6 of the
 * fighter's (g7) first word is set), and three words are copied from
 * src + offset to dst + offset for each. */
#pragma once

static uint32_t gfn_unit_smooth_cancel_rear(int32_t entry, int32_t dst, int32_t src, uint32_t mask)
{
    uint32_t groups = (mask & 0x1E000u) >> 13;
    uint32_t table = (gems_ld32(GEMS_G(7)) & 0x40u) ? 0x2FD74u : 0x2FD64u;
    for (;;) {
        int32_t g = -1;   /* the highest group bit left (scanbit) */
        for (int b = 31; b >= 0; b--)
            if (groups & (1u << b)) { g = b; break; }
        if (g < 0) break;
        for (uint32_t k = 0; k < 3u; k++) {
            uint32_t off = gems_ld8(table + (uint32_t)g * 4u + k);
            for (uint32_t w = 0; w < 12u; w += 4u)
                gems_st32((uint32_t)dst + off + w, gems_ld32((uint32_t)src + off + w));
        }
        groups &= ~(1u << g);
    }
    if (entry) gems_i960_ret();
    return 0;
}
