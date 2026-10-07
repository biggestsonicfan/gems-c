#pragma once
/* COP op 18 Fn_cvtsw */

/* PowerPC fctiwz: truncate toward zero, saturate, NaN -> 0x80000000. */
static inline uint32_t gems18_fctiwz(float f) {
    if (f != f) return 0x80000000u;
    if (f >= 2147483648.0f) return 0x7FFFFFFFu;
    if (f < -2147483648.0f) return 0x80000000u;
    return (uint32_t)(int32_t)f;
}

static void gcop_18(void) {
    gch_80022e7c(gems18_fctiwz(gems_in_f()));
}
