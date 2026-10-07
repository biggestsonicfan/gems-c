/* os_set_osage_after (i960 0x68830; Gems 8004E49C): draw a sway segment
 * (osage record g9) through set_obj_fifo, whose 12 words come from Fn_osage's
 * reply. Model 0 for character 4 in motion 0x169 / 0x16A or with rob+0x84C
 * == 3, else the record's model (+0x18). g2 = bit 5 of g13's flags, plus 1
 * when the model is 0. If set_obj_fifo moved the FIFO pointer at 0x50E400,
 * one more word (0, or the record's +0x1C) is appended. Then g13+0xF8 takes
 * the word at g8, and with g13+6 below 2 the three words at g8+0xC clear. */
#pragma once

static uint32_t gfn_os_set_osage_after(int entry)
{
    uint32_t chr = gems_ld8(GEMS_G(7) + 0x1B1);
    int blank = 0;
    if (chr == 4) {
        if (gems_ld16(GEMS_G(7) + 0x1A8) == 0x169 ||
            gems_ld16(GEMS_G(7) + 0x1A8) == 0x16A ||
            gems_ld8(GEMS_G(7) + 0x84C) == 3)
            blank = 1;
    }
    GEMS_G(3) = gems_ld32(0x50E400);
    GEMS_G(2) = gems_ld32(GEMS_G(13));
    if (blank) GEMS_G(0) = 0;
    else GEMS_G(0) = gems_ld32(GEMS_G(9) + 0x18);
    GEMS_G(2) = (GEMS_G(2) >> 5) & 1;
    GEMS_G(1) = gems_ld8(GEMS_G(7) + 4);
    GEMS_G(2) |= GEMS_G(0) == 0;
    uint32_t fifo = GEMS_G(3);
    gfn_set_obj_fifo(0);
    if (GEMS_G(3) != fifo) {
        gems_st32(GEMS_G(3), blank ? 0 : gems_ld32(GEMS_G(9) + 0x1C));
        GEMS_G(3) += 4;
        gems_st32(0x50E400, GEMS_G(3));
    }
    gems_st32(GEMS_G(13) + 0xF8, gems_ld32(GEMS_G(8)));
    if (gems_ld16(GEMS_G(13) + 6) < 2) {
        static const uint32_t zero[3] = { 0, 0, 0 };
        gems_stn(GEMS_G(8) + 0xC, zero, 3);
    }
    if (entry) gems_i960_ret();
    return 0;
}
