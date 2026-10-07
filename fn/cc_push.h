/* cc_push (i960 0x3C458; Gems 80051D3C): the CPU command "push": the 32-bit
 * operand after the command byte (little-endian bytes at g6+1..4) goes to
 * g4+0x114, g6 steps over the command, and the i960 carries on at the jump
 * back to sec_cc_next (site + 0x30). */
#pragma once

static uint32_t gfn_cc_push(int entry)
{
    (void)entry;
    uint32_t cmd = GEMS_G(6);
    GEMS_R(3)  = gems_ld8(cmd + 1);
    GEMS_R(15) = gems_ld8(cmd + 2) << 8;
    GEMS_R(3) += GEMS_R(15);
    GEMS_R(15) = gems_ld8(cmd + 3) << 16;
    GEMS_R(3) += GEMS_R(15);
    GEMS_R(15) = gems_ld8(cmd + 4) << 24;
    GEMS_R(3) += GEMS_R(15);
    gems_st32(GEMS_G(4) + 0x114, GEMS_R(3));
    GEMS_G(6) = cmd + 5;
    return 0x60;
}
