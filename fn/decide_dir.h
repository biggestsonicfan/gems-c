/* decide_dir (i960 0x2D588; Gems 80042E94): the direction between the two
 * fighters for the camera (g13). g7/g8 = the two fighters (0x500804/0x500808).
 * Unless either fighter has flag 0x20000000 or 0x40000 in rob+0x1A4, it first
 * picks a follow rate (0.4; 0.3 when one of them is in a throw, rob+0x804
 * bit 8, of kind 5 (rob+0x821); else 0.85 when fighter 1 has flag 0x4000;
 * 0 unless rob+0x5F4 < rob+0x7D4), stores it at g13+0x270 and stops there
 * unless it is <= g13+0x168. Then g13+0xD4..0xDC = the unit vector
 * (COP 0x30) from fighter 1 to fighter 2 in x/z, or zeros when they stand on
 * the same spot. */
#pragma once

static uint32_t gfn_decide_dir(int entry)
{
    GEMS_G(7) = gems_ld32(0x500804u);
    GEMS_G(8) = gems_ld32(0x500808u);
    uint32_t p1 = GEMS_G(7), p2 = GEMS_G(8), cam = GEMS_G(13);
    uint32_t rate = 0x3ECCCCCDu;                     /* 0.4f */
    uint32_t flags1 = gems_ld32(p1 + 0x1A4u);
    uint32_t flags = flags1 | gems_ld32(p2 + 0x1A4u);

    if ((flags & 0x20000000u) == 0 && (flags & 0x40000u) == 0) {
        uint32_t throw1 = gems_ld32(p1 + 0x804u);
        uint32_t throw2 = gems_ld32(p2 + 0x804u);
        bool kind5 = false;
        if ((throw1 | throw2) & 0x100u) {
            uint32_t kind1 = gems_ld8(p1 + 0x821u);
            uint32_t kind2 = gems_ld8(p2 + 0x821u);
            if (throw1 & 0x100u)
                kind5 = kind1 == 5u;
            else if (throw2 & 0x100u)
                kind5 = kind2 == 5u;
        }
        if (kind5)
            rate = 0x3E99999Au;                      /* 0.3f */
        else if (flags1 & 0x4000u)
            rate = 0x3F59999Au;                      /* 0.85f */
        if (!(gems_ldf(p1 + 0x5F4u) < gems_ldf(p1 + 0x7D4u)))
            rate = 0;
        gems_st32(cam + 0x270u, rate);
        if (gems_u2f(rate) > gems_ldf(cam + 0x168u))       /* cmpr + bg: NaN goes on */
            goto done;
    }
    {
        uint32_t a[3], b[3], v[3];
        gems_ldn(a, p1 + 0x1F4u, 3);
        gems_ldn(b, p2 + 0x1F4u, 3);
        v[0] = gems_f2u(gems_u2f(b[0]) - gems_u2f(a[0]));
        v[1] = 0;
        v[2] = gems_f2u(gems_u2f(b[2]) - gems_u2f(a[2]));
        if (((v[0] | v[2]) & 0x7FFFFFFFu) == 0) {
            v[0] = v[1] = v[2] = 0;
        } else {
            gems_cop_w(0x18003030u);                 /* normalise */
            gems_cop_wn(v, 3);
            gems_cop_rn(v, 3);
        }
        gems_st32(cam + 0xD4u, v[0]);
        gems_st32(cam + 0xD8u, v[1]);
        gems_st32(cam + 0xDCu, v[2]);
    }
done:
    if (entry) gems_i960_ret();
    return 0;
}
