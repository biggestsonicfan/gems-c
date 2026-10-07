/* cage_clip_m (i960 0x24738): the cage stage's fence posts. Projects each
 * post of the current cage side (g4) to the screen and draws the segment
 * between two posts unless both lie off the same side. g2 walks the
 * stage's per-segment object list. */

/* One post through 0x14802929 (the projection); its outcode: 31 when z is
 * not past 2.0, else bit 0 left of -248 (or NaN), bit 1 right of 248. */
static uint32_t gfn_cage_clip_m__outcode(float sx)
{
    uint32_t px = gems_ld32(GEMS_G(3));
    GEMS_G(3) += 4;
    gems_cop_w(0x14802929);
    gems_cop_w(px);
    gems_cop_w(0);
    gems_cop_w(0x40C00000);             /* 6.0 */
    float x = gems_cop_rf();
    (void)gems_cop_r();                 /* y: Gems only uses it on the first post */
    float z = gems_cop_rf();
    x = (sx * x) / z;
    if (!(z > 2.0f))
        return 31;
    uint32_t code = (x > -248.0f) ? 0 : 1;
    if (x >= 248.0f)
        code |= 2;
    return code;
}

/* translate (0,0,6), draw obj, translate back (0,0,-6) */
static void gfn_cage_clip_m__draw(uint32_t obj, uint32_t ty, uint32_t ty_back)
{
    gems_cop_w(0x03000606);
    gems_cop_w(0);
    gems_cop_w(ty);
    gems_cop_w(0x40C00000);
    GEMS_G(1) = 0;
    GEMS_G(0) = obj;
    gfn_set_obj();
    gems_cop_w(0x03000606);
    gems_cop_w(0);
    gems_cop_w(ty_back);
    gems_cop_w(0xC0C00000);
}

static uint32_t gfn_cage_clip_m(int entry)
{
    uint32_t side[2];

    gems_cop_w(0x00800101);
    gems_ldn(side, 0x903D0 + GEMS_G(4) * 8, 2);
    gems_cop_w(0x03000606);
    gems_cop_w(0);
    gems_cop_w(0);
    gems_cop_w(side[0]);

    uint32_t rec = gems_ld8(0x500064) * 0x100;
    uint32_t flags = gems_ld32(rec + 0x8F3D0);
    GEMS_G(2) = rec + 0x8F454 + side[1] * 0x10;
    if (flags & 0x100000) {
        gems_cop_w(0x03000606);
        gems_cop_w(0);
        gems_cop_w(0);
        gems_cop_w(0x40C00000);
        GEMS_G(1) = 0;
        GEMS_G(0) = gems_ld16(rec + 0x8F3F0);               /* ldos */
        gfn_set_obj();
        gems_cop_w(0x03000606);
        gems_cop_w(0x80000000);
        gems_cop_w(0x80000000);
        gems_cop_w(0xC0C00000);
    }

    float sx = gems_ldf(0x501084);      /* screen scale x (ldl; y unused) */
    uint32_t n = 8;
    GEMS_G(3) = 0x24C80;
    if (flags & 0x80000)    { GEMS_G(3) = 0x24CA4; n = 4; }
    if (flags & 0x200000)   { GEMS_G(3) = 0x24CB8; n = 2; }
    if (flags & 0x20000000) { GEMS_G(3) = 0x24CC4; n = 3; }
    if (flags & 0x400000)   { GEMS_G(3) = 0x24CD4; n = 1; }

    uint32_t prev = gfn_cage_clip_m__outcode(sx);
    do {
        uint32_t cur = gfn_cage_clip_m__outcode(sx);
        if ((prev & cur) == 0) {
            uint32_t stage = gems_ld8(0x500064);
            if (stage == 4) {
                uint32_t obj = gems_ld32(0x24AAC + GEMS_G(1) * 4);
                gems_cop_w(0x00800101);
                gfn_cage_clip_m__draw(obj, 0, 0);
                gems_cop_w(0x01000202);
            } else if (stage == 1) {
                uint32_t t = gems_ld32(0x500020);
                gfn_cage_clip_m__draw(gems_ld32(0x90E8C + ((t & 0x7F) >> 1) * 4), 0, 0);
            } else if (stage == 10
                       || (stage == 9 && (gems_ld32(0x500498) & 4))) {
                uint32_t t = gems_ld32(0x500020);
                gfn_cage_clip_m__draw(gems_ld16(0x90DEC + (t & 0x3F) * 2), 0, 0);
            } else if (stage == 9) {
                gems_cop_w(0x0B801717);
                gems_cop_w(gems_ld16(0x50049E));            /* ldos */
                float y = gems_cop_rf() * -0.1f;
                gfn_cage_clip_m__draw(0xD2D, gems_f2u(y), gems_f2u(y) ^ 0x80000000u);
            } else {
                gfn_cage_clip_m__draw(gems_ld16(GEMS_G(2)), 0, 0); /* ldos */
            }
        }
        prev = cur;
        GEMS_G(2) += 2;
    } while (--n != 0);

    gems_cop_w(0x01000202);
    if (entry)
        gems_i960_ret();
    return 0;
}
