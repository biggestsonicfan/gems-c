#pragma once
/* COP op 62 Fn_calc_unit_hara */

/* Push, put T, build the 3x3 from three angles in place, then turn it by
 * three more (Y, X, Z). */
static void gcop_62(void) {
    float tx = gems_in_f(), ty = gems_in_f(), tz = gems_in_f();
    uint32_t a0 = (uint16_t)gems_in_w();
    uint32_t a1 = (uint16_t)gems_in_w();
    uint32_t a2 = (uint16_t)gems_in_w();
    uint32_t a3 = (uint16_t)gems_in_w();
    uint32_t a4 = (uint16_t)gems_in_w();
    uint32_t a5 = (uint16_t)gems_in_w();

    /* push: depth (+0xCF0) below 7 moves the current index on a matrix */
    if (*gems_dm(0x3033Cu) < 7u) {
        *gems_dm(0x3033Cu) += 1u;
        *gems_dm(0x3033Fu) += 12u;
    }
    float *t = gch_cur();
    t[9] = tx; t[10] = ty; t[11] = tz;

    float *m = gch_cur();
    float s0, c0, s1, c1, s2, c2;
    gch_8001ead8(&s0, &c0, a0);
    gch_8001ead8(&s1, &c1, a1);
    gch_8001ead8(&s2, &c2, a2);
    m[0] = c0 * c1;
    m[1] = -(s0 * c1);
    m[3] = fmaf(s0, c2, s2 * (c0 * s1));
    m[6] = fmaf(s0, s2, -(c2 * (c0 * s1)));
    m[2] = s1;
    m[4] = fmaf(c0, c2, -(s1 * (s0 * s2)));
    /* The GC's order (0x80023c2c), kept on purpose.  The board (cpres1.asm
     * _L211C0, PM 0x211C0) multiplies (s0*s1)*c2, and m[3], m[4] and m[6]
     * agree with it, but it also rounds both products and the add separately,
     * toward zero.  The board's order alone would still not give its bits. */
    m[7] = fmaf(c0, s2, s1 * (c2 * s0));
    m[8] = c1 * c2;
    m[5] = -(c1 * s2);

    gch_8001e808(a3, *gems_dm(0x3033Fu));
    gch_8001e8b4(a4, *gems_dm(0x3033Fu));
    gch_8001e6f8(a5, *gems_dm(0x3033Fu));
}
