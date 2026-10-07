/* get_fcurve_value_f (i960 0x30C28, Gems 80040248): sample the 60 motion
 * channels of the fighter g7 at frame g6 (a float, compared as a word) into
 * the 60 words at g5.
 *
 * The motion record (+0xBD8) holds per channel a type byte (+0x78C), and
 * three streams: values (+0x780 -> g2), key counts (+0x784 -> g0) and key
 * times (+0x788 -> g3). Type 3: 0. Types 0-2: nothing, one value skipped.
 * Type 4: a constant. Type 5: linear keys (one value a key) through 0x31
 * (Fn_fcurve_lin). Type 6+: spline keys (value, in and out tangent) through
 * 0x32 (Fn_fcurve_spl). A frame exactly on a key takes the key's value.
 *
 * Then the first 36 words (the angles) are cut to 16-bit integers in place
 * (cvtri, rounded by the AC mode; out of range or NaN gives 0x80000000;
 * stored with stis), and the
 * mirrored fighter (bit 6 of [g7]) goes through set_mirror. Leaves g0-g6 as
 * the i960 does (g1 0, g5 back at the start). */

static uint32_t gfn_get_fcurve_value_f(int entry)
{
    uint32_t mot = gems_ld32(GEMS_G(7) + 0xBD8);
    GEMS_G(4) = mot + 0x78C;
    GEMS_G(3) = gems_ld32(mot + 0x788);
    GEMS_G(2) = gems_ld32(mot + 0x780);
    GEMS_G(1) = 60;
    GEMS_G(0) = gems_ld32(mot + 0x784);

    /* The previous key's time (the i960's r7). It carries over from channel
     * to channel; a frame before a channel's first key would read it. The
     * call that enters here zeroes the frame's locals, so it starts at 0. */
    uint32_t prev = 0;
    do {
        uint32_t frame = GEMS_G(6);
        uint32_t times = GEMS_G(3);
        uint32_t type  = gems_ld8(GEMS_G(4));   /* ldob */
        if (type == 4) {
            gems_st32(GEMS_G(5), gems_ld32(GEMS_G(2)));
            GEMS_G(2) += 4;
        } else if (type < 4) {
            if (type == 3) gems_st32(GEMS_G(5), 0);
            else           GEMS_G(2) += 4;
        } else {
            uint32_t keys = gems_ld8(GEMS_G(0));   /* ldob */
            GEMS_G(0) += 1;
            uint32_t stride = type == 5 ? 4u : 12u;
            uint32_t k = 0, key;
            bool found = false;
            do {
                key = gems_ld32(times);
                times += 4;
                if (frame <= key) { found = true; break; }   /* cmpoble: unsigned */
                k++;
                prev = key;
            } while (keys > k);
            GEMS_G(3) += keys * 4;
            uint32_t p = GEMS_G(2) + k * stride;
            GEMS_G(2) += keys * stride;
            if (found) {
                if (frame == key) {
                    gems_st32(GEMS_G(5), gems_ld32(p));
                } else if (type == 5) {
                    float span = gems_u2f(key) - gems_u2f(prev);     /* fsubs */
                    float t    = gems_u2f(frame) - gems_u2f(prev);   /* fsubs */
                    uint32_t v1 = gems_ld32(p);
                    uint32_t v0 = gems_ld32(p - 4);
                    gems_cop_w(0x18803131u);
                    gems_cop_w(v0);
                    gems_cop_w(v1);
                    gems_cop_wf(t);
                    gems_cop_wf(span);
                    gems_st32(GEMS_G(5), gems_cop_r());
                } else {
                    float span = gems_u2f(key) - gems_u2f(prev);     /* fsubs */
                    float t    = gems_u2f(frame) - gems_u2f(prev);   /* fsubs */
                    uint32_t v0 = gems_ld32(p - 12);
                    uint32_t v1 = gems_ld32(p);
                    uint32_t m0 = gems_ld32(p - 4);
                    uint32_t m1 = gems_ld32(p + 4);
                    prev = m0;   /* the i960 loads the tangent into r7 */
                    gems_cop_w(0x19003232u);
                    gems_cop_wf(span);
                    gems_cop_wf(t);
                    gems_cop_w(v0);
                    gems_cop_w(v1);
                    gems_cop_w(m0);
                    gems_cop_w(m1);
                    gems_st32(GEMS_G(5), gems_cop_r());
                }
            }
        }
        GEMS_G(4) += 1;
        GEMS_G(5) += 4;
    } while (GEMS_G(1)-- > 1);

    GEMS_G(5) -= 240;
    uint32_t n = 36;
    do {
        gems_st16(GEMS_G(5), gems_cvtri(gems_ld32(GEMS_G(5))));
        GEMS_G(5) += 4;
    } while (n-- > 1);
    GEMS_G(5) -= 144;

    if (gems_ld32(GEMS_G(7)) & 0x40u)
        gfn_set_mirror(0);
    if (entry) gems_i960_ret();
    return 0;
}
