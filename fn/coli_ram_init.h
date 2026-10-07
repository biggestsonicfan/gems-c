/* coli_ram_init (i960 0x2D534; Gems 80045068): clear the collision work at
 * g13 (+0x170..+0x1AF, +0x98..+0xAB) and each fighter's +0x75C/+0x75E halves
 * and the pair at +0xA68. */
#pragma once

static uint32_t gfn_coli_ram_init(int entry)
{
    static const uint32_t zero[4] = { 0, 0, 0, 0 };
    gems_stn(GEMS_G(13) + 0x170, zero, 4);
    gems_stn(GEMS_G(13) + 0x180, zero, 4);
    gems_stn(GEMS_G(13) + 0x190, zero, 4);
    gems_stn(GEMS_G(13) + 0x1A0, zero, 4);
    for (int p = 0; p < 2; p++) {
        uint32_t rob = gems_ld32(p ? 0x500808u : 0x500804u);
        gems_st16(rob + 0x75C, 0);
        gems_st16(rob + 0x75E, 0);
        gems_stn(rob + 0xA68, zero, 2);
    }
    for (uint32_t o = 0x98; o <= 0xA8; o += 4)
        gems_st32(GEMS_G(13) + o, 0);
    if (entry) gems_i960_ret();
    return 0;
}
