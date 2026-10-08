#pragma once
/* COP op 69 Fn_mul_mot_yrot */

/* A 3x3 from bufferram (staged at DM 0x30410), turned about Y, becomes the
 * current 3x3. */
static void gcop_69(void) {
    uint32_t b   = (uint16_t)gems_in_w();
    uint32_t ang = (uint16_t)gems_in_w();
    for (uint32_t k = 0; k < 9; k++)
        *gems_dm(0x30410u + k) = gems_bram_rd((uint16_t)(b + k));
    const float *t = gems_dmf(0x30410u);
    float *m = gch_cur();
    float s, c;
    gch_8001ead8(&s, &c, ang);
    m[0] = c * t[0] - s * t[2];
    m[3] = c * t[3] - s * t[5];
    m[6] = c * t[6] - s * t[8];
    m[2] = s * t[0] + c * t[2];
    m[5] = s * t[3] + c * t[5];
    m[8] = s * t[6] + c * t[8];
    m[1] = t[1];
    m[4] = t[4];
    m[7] = t[7];
}
