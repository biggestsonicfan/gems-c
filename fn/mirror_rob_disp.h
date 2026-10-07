/* mirror_rob_disp (i960 0x641C0; Gems 8004FD6C): the fighters' reflections
 * (rob_disp_mir) under a matrix mirrored in Y, for each fighter whose bit is
 * set at *(0x500814) and clear at 0x50009C, P2 only when the stage asks
 * (g13+0xDE bit 0). Each one's polygon count goes to 0x500178 / 0x50017A.
 *
 * Gems brackets it with FUN_8002F884(1) / (0), the GameCube renderer's
 * mirror flag; the board has none (the mirror is the COP's scale matrix), so
 * they are gone. */
#pragma once

static uint32_t gfn_mirror_rob_disp(int entry)
{
    gems_cop_w(0x00800101u);            /* Fn_push_matrix */
    gems_cop_w(0x03800707u);            /* Fn_scale (1, -1, 1) */
    gems_cop_w(0x3F800000u);
    gems_cop_w(0xBF800000u);
    gems_cop_w(0x3F800000u);

    uint32_t off   = gems_ld8(0x50009C);
    uint32_t shown = gems_ld32(0x500814);
    if (!(off & 2u) && (gems_ld32(shown) & 2u)) {
        GEMS_G(7) = gems_ld32(0x500804);
        uint32_t before = gems_ld32(0x50101C);
        gfn_rob_disp_mir(0);
        gems_st16(0x500178, gems_ld32(0x50101C) - before);
    }

    /* The last bit test leaves the condition code unless rob_disp_mir ran. */
    if (!(gems_ld8(GEMS_G(13) + 0xDE) & 1u)) {
        GEMS_AC = (GEMS_AC & ~7u) | 0u;
    } else if (gems_ld8(0x50009C) & 4u) {
        GEMS_AC = (GEMS_AC & ~7u) | 2u;
    } else if (!(gems_ld32(shown) & 4u)) {
        GEMS_AC = (GEMS_AC & ~7u) | 0u;
    } else {
        GEMS_G(7) = gems_ld32(0x500808);
        uint32_t before = gems_ld32(0x50101C);
        gfn_rob_disp_mir(0);
        gems_st16(0x50017A, gems_ld32(0x50101C) - before);
    }

    gems_cop_w(0x01000202u);            /* Fn_pop_matrix */
    if (entry) gems_i960_ret();
    return 0;
}
