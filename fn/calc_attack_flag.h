/* calc_attack_flag (i960 0x2DC48; Gems 800451F8): which of the fighter's (g7)
 * 32 units the current attack hits with. When the attack bit (0x100 of
 * rob+0x1A4) is up, rob+0x1AA has reached rob+0x808 and the attack kind
 * (byte rob+0x820) is not 0x29, the kind's part mask (table 0xCE020) is
 * tested at each unit's part number (rob+0x1D18 points at 32 words), giving
 * the unit mask g3. Stores the mask word at rob+0xAC8 (0 when the bit is down
 * or the time not reached, the kind itself when it is 0x29) and g3 at
 * rob+0x768. */
#pragma once

static uint32_t gfn_calc_attack_flag(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t mask = 0;
    GEMS_G(3) = 0;
    if (gems_ld32(rob + 0x1A4u) & 0x100u) {
        if (gems_ld16(rob + 0x1AAu) >= gems_ld16(rob + 0x808u)) {
            mask = gems_ld8(rob + 0x820u);
            if (mask != 0x29u) {
                mask = gems_ld32(0xCE020u + mask * 4u);
                uint32_t parts = gems_ld32(rob + 0x1D18u);
                for (uint32_t unit = 0; unit < 32u; unit++) {
                    uint32_t part = gems_ld32(parts + unit * 4u);
                    /* PowerPC slw: a shift of 32-63 gives 0 */
                    uint32_t bit = (part & 32u) ? 0u : 1u << (part & 31u);
                    if (mask & bit) GEMS_G(3) |= 1u << unit;
                }
            }
        }
    }
    gems_st32(rob + 0xAC8u, mask);
    gems_st32(rob + 0x768u, GEMS_G(3));
    if (entry) gems_i960_ret();
    return 0;
}
