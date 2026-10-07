/* area_clip (i960 0x28334, Gems 8004F9BC): draw each ground chunk of the
 * list at g2 (count, then four corner offsets per chunk) whose corners'
 * outcodes at 0x50E000 AND to zero, model numbers from g3; then the chunk
 * under the camera (0x35 on the origin; 160-unit cells, 4 x 4) if it was not
 * drawn already. Leaves g2 past the list. */

/* i960 setbit / bbs: the bit number is taken mod 32. */
static inline uint32_t area_clip_bit(uint32_t n)
{
    return 1u << (n & 0x1F);
}

static uint32_t gfn_area_clip(int entry)
{
    const uint32_t outcodes = 0x50E000;
    uint32_t drawn = 0;
    uint32_t last  = gems_ld16(GEMS_G(2));                       /* ldos */
    GEMS_G(2) += 2;
    uint32_t i = 0;
    bool more;
    do {
        uint32_t g2 = GEMS_G(2);
        uint32_t code = gems_ld16(outcodes + gems_ld16(g2));
        code &= gems_ld16(outcodes + gems_ld16(g2 + 2));
        code &= gems_ld8(outcodes + gems_ld16(g2 + 4));
        code &= gems_ld8(outcodes + gems_ld16(g2 + 6));
        GEMS_G(2) += 8;
        if (code == 0) {
            uint32_t model = gems_ld16(GEMS_G(3) + i * 2);
            if (model != 0) {
                GEMS_G(1) = 0;
                GEMS_G(0) = model;
                gfn_set_obj();
                drawn |= area_clip_bit(i);
            }
        }
        more = last > i;
        i++;
    } while (more);

    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    (void)gems_cop_r();
    gems_cop_w(0x35006A6A);
    gems_cop_w(0);
    gems_cop_w(0);
    gems_cop_w(0);
    float x = gems_cop_rf();
    (void)gems_cop_r();
    float z = gems_cop_rf();
    uint32_t cx = gems_cvtri(gems_f2u(x + 320.0f)) / 160 & 3;   /* addr, cvtri, divo */
    uint32_t cz = gems_cvtri(gems_f2u(z + 320.0f)) / 160 & 3;
    uint32_t cell = cx + cz * 4;
    if (!(drawn & area_clip_bit(cell))) {
        GEMS_G(1) = 0;
        GEMS_G(0) = gems_ld16(GEMS_G(3) + cell * 2);
        gfn_set_obj();
    }
    if (entry) gems_i960_ret();
    return 0;
}
