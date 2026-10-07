/* clip_point_check_yoko (i960 0x28188; Gems 8004F440): outcodes for a point
 * list. g1 points at a count, then (x, z) pairs; each goes through COP 0x29
 * (y = 0) and gets one byte at 0x50E000 on: 0x90 behind the lens (w <= 1),
 * 0x81 left of x = -248, 0x82 right of x = +248, in screen units scaled by
 * 2/3 of the projection at 0x501084. area_clip ANDs them (CLAUDE.md, "Never
 * hook clip_point_check_yoko"). g1 ends past the list, g5 at -1. */
#pragma once

static uint32_t gfn_clip_point_check_yoko(int entry)
{
    uint32_t proj[2];
    gems_ldn(proj, 0x501084, 2);
    const float k = 0.6667f;              /* 0x3F2AACDA */
    float sx = k * gems_u2f(proj[0]);
    float sy = k * gems_u2f(proj[1]);
    uint32_t out = 0x50E000;
    GEMS_G(5) = gems_ld32(GEMS_G(1));
    GEMS_G(1) += 4;
    for (;;) {
        uint32_t pt[2];
        gems_ldn(pt, GEMS_G(1), 2);
        GEMS_G(1) += 8;
        gems_cop_w(0x14802929u);
        gems_cop_w(pt[0]);
        gems_cop_w(0);
        gems_cop_w(pt[1]);
        float cx = gems_cop_rf();
        float cy = gems_cop_rf();
        float w  = gems_cop_rf();
        float x  = (sx * cx) / w;
        (void)((sy * cy) / w);            /* Gems works out y too; unused */
        uint32_t code = 0;
        if (!(w > 1.0f)) {
            code = 0x90;
        } else {
            if (!(x > -248.0f)) code |= 0x81;
            if (x >= 248.0f)    code |= 0x82;
        }
        gems_st8(out++, code);
        uint32_t n = GEMS_G(5);
        GEMS_G(5) = n - 1;
        if (n <= 1) break;
    }
    if (entry) gems_i960_ret();
    return 0;
}
