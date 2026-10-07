/* efc_mabuta_cont (i960 0x32F24; Gems 800486DC): eyelid blinks. Off in game
 * modes 0xE / 0xF, unless the stage flags (0x500814) have bits 1 and 2 up,
 * or with rob+0x1A4 bit 8: then the blink state (rob+0x2A92 flags,
 * +0x2A93 frame, +0x2A94 model) clears. A blink starts on a 1-in-16-ish
 * rand (low nibble above 6), runs through the character's lid table
 * (0x34C0E[rob+0x1B0]) for 10 frames, drawing each lid model; Sonic (2) and
 * Knuckles (28) also swap their face model at rob+0x48 while it runs. */
#pragma once

/* The face model for characters 2 and 28 (0x34E86 / 0x34E8E), by facing
 * (rob flags bit 1); `idx` 0 while blinking, 4 when the lids are open. */
static void efc_mabuta_cont_face(uint32_t chr, uint32_t idx)
{
    uint32_t tab;
    if (chr == 2) tab = 0x34E86;
    else if (chr == 28) tab = 0x34E8E;
    else return;
    uint32_t face;
    if (gems_ld32(GEMS_G(7)) & 2) face = gems_ld16(tab + idx);
    else face = gems_ld16(tab + idx + 2);
    gems_st16(GEMS_G(7) + 0x48, face);
}

static uint32_t gfn_efc_mabuta_cont(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t mode = gems_ld8(0x50002B);
    uint32_t stage = gems_ld32(0x500814);
    if (mode == 0xE || mode == 0xF ||
        !(gems_ld32(stage) & 2) || !(gems_ld32(stage) & 4) ||
        (gems_ld32(rob + 0x1A4) & 0x100)) {
        gems_st16(rob + 0x2A92, 0);
        gems_st16(rob + 0x2A94, 0);
        if (entry) gems_i960_ret();
        return 0;
    }
    uint32_t flags = gems_ld8(rob + 0x2A92);
    int blink = 0;
    if (flags & 4) {
        blink = 1;
    } else if (flags & 2) {
        gfn_rand(0);
        if ((GEMS_G(0) & 0xF) > 6) {
            gems_st8(rob + 0x2A92, flags | 4);
            blink = 1;
        }
    } else {
        if (entry) gems_i960_ret();
        return 0;
    }
    if (blink) {
        uint32_t frame = gems_ld8(rob + 0x2A93);
        if (frame != 10) {
            uint32_t chr = gems_ld8(rob + 0x1B0);
            uint32_t lids = gems_ld32(chr * 4 + 0x34C0E);
            if (lids != 0) {
                uint32_t model = gems_ld16(lids + frame * 2);
                gems_st16(rob + 0x2A94, model);
                GEMS_G(1) = 0;
                GEMS_G(0) = model;
                gfn_set_obj();
                efc_mabuta_cont_face(chr, 0);
                if (!(gems_ld32(0x508000) & 0x20))
                    gems_st8(rob + 0x2A93, frame + 1);
                if (entry) gems_i960_ret();
                return 0;
            }
        }
    }
    /* the blink is over (or never started): clear it, lids open */
    gems_st16(rob + 0x2A92, 0);
    gems_st16(rob + 0x2A94, 0);
    efc_mabuta_cont_face(gems_ld8(rob + 0x1B0), 4);
    if (entry) gems_i960_ret();
    return 0;
}
