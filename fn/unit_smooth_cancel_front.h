/* unit_smooth_cancel_front (i960 0x2FC58, Gems 0x8003F6E8): when exactly one
 * of the two motion flag words (rob+0xC5C, rob+0x860) has bit 12 set, hand
 * the other one's flags to unit_smooth_cancel_rear; otherwise return. */
#pragma once

static uint32_t gfn_unit_smooth_cancel_front(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t flags = gems_ld32(rob + 0xC5C);
    if (!(flags & 0x1000)) {
        flags = gems_ld32(rob + 0x860);
        if (!(flags & 0x1000)) {
            if (entry) gems_i960_ret();
            return 0;
        }
    } else if (gems_ld32(rob + 0x860) & 0x1000) {
        if (entry) gems_i960_ret();
        return 0;
    }
    gfn_unit_smooth_cancel_rear(entry, 0x50E0F0, 0x50E000, flags);
    return 0;
}
