#pragma once
/* COP op 80 Fn_zanzou_reserve */

/* The one COP command whose length the opcode does not give: header (player,
 * part mask, life step, spacing), then per part index + three object numbers,
 * then -1 and the turn angle. Gems reads it in one go and replies one word (0)
 * at the end; here it is fed a word at a time and also answers each part, as
 * the firmware does. Word offsets are from DM 0x30000. */
static struct {
    uint32_t stage;      /* 0..3 header word, 4 part index, 5..7 object words, 8 turn */
    uint32_t player, mask, life_step;
    float    spacing;
    uint32_t obj, life, mask_word, src, prev;
    uint32_t part, sub;
} g80;

static void gcop_zanzou_begin(void) {
    g80.stage = 0;
    g80.player = g80.mask = g80.life_step = 0;
    g80.spacing = 0.0f;
    g80.obj = g80.life = g80.mask_word = g80.src = g80.prev = 0;
    g80.part = g80.sub = 0;
}

static void g80_finish(uint32_t turn) {
    uint32_t *S  = gems_dm(0x30000u);
    float    *Sf = gems_dmf(0x30000u);
    const float *src  = Sf + g80.src;
    const float *prev = Sf + g80.prev;
    float sp = g80.spacing;

    /* the furthest any marked part (or its point 'spacing' along col0..2) moved */
    float max = 0.0f;
    for (uint32_t k = 0; k < 16; k++) {
        if (!(g80.mask & (1u << k))) continue;
        const float *c = src + 12u * k;
        const float *p = prev + 12u * k;
        float d10 = c[10] - p[10];
        float d11 = c[11] - p[11];
        float d9  = c[9] - p[9];
        d10 = d10 * d10;
        d11 = d11 * d11;
        d9  = d9 * d9;
        float d = d9 + (d11 + d10);
        if (d >= max) max = d;

        float e10 = (sp * c[1] + c[10]) - (sp * p[1] + p[10]);
        float e11 = (sp * c[2] + c[11]) - (sp * p[2] + p[11]);
        float e9  = (sp * c[0] + c[9]) - (sp * p[0] + p[9]);
        e10 = e10 * e10;
        e11 = e11 * e11;
        e9  = e9 * e9;
        d = e9 + (e11 + e10);
        if (d >= max) max = d;
    }
    if (!(13.0f > max)) return;   /* a teleport lays nothing */

    float step = gch_8001e174(max);
    step = gems_u2f(S[0x2181]) * step;
    if (0.0f == step) step = 1.0f;
    if (0.01f >= step) step = 0.01f;
    for (uint32_t k = 0; k < 16; k++) {
        if (!(g80.mask & (1u << k)))
            S[g80.life + k] = 0;
        else
            gch_80021d88(step, src, prev, g80.obj, g80.life, turn, g80.life_step, g80.player, k);
    }

    /* turn the fresh copies (flag bit 31) about Y */
    float s, c;
    gch_8001ead8(&s, &c, turn & 0xFFFFu);
    for (uint32_t j = 0; j < 128; j++) {
        uint32_t w = 0x2301u + 32u * j;
        uint32_t f = S[w];
        if (!(f & 0x80000000u)) continue;
        S[w] = f & 1u;
        float *q = Sf + w + 22u;
        float q0 = q[0], q1 = q[1], q2 = q[2];
        q[0] = q0 * c - q[3] * s;
        q[3] = q0 * s + q[3] * c;
        q[1] = q1 * c - q[4] * s;
        q[4] = q1 * s + q[4] * c;
        q[2] = q2 * c - q[5] * s;
        q[5] = q2 * s + q[5] * c;
    }
}

/* Returns true on the word that completes the stream (the turn angle). */
static bool gcop_zanzou_feed(uint32_t w) {
    uint32_t *S = gems_dm(0x30000u);
    switch (g80.stage) {
    case 0:
        g80.player = w;
        S[0x12] = w;
        if (w == 1u) {
            g80.obj = 0x2200u; g80.life = 0x21F0u; g80.mask_word = 0x2240u;
            g80.src = 0x4E0u;  g80.prev = 0x20C0u;
        } else {
            g80.obj = 0x21A0u; g80.life = 0x2190u; g80.mask_word = 0x21E0u;
            g80.src = 0x420u;  g80.prev = 0x2000u;
        }
        g80.stage = 1;
        return false;
    case 1:
        g80.mask = w;
        g80.stage = 2;
        return false;
    case 2:
        g80.life_step = w;
        g80.stage = 3;
        return false;
    case 3: {
        g80.spacing = gems_u2f(w);
        /* parts dropped from or added to the mask lose their timers */
        uint32_t changed = g80.mask ^ S[g80.mask_word];
        for (uint32_t k = 0; k < 16; k++)
            if (changed & (1u << k)) S[g80.life + k] = 0;
        S[g80.mask_word] = g80.mask;
        g80.stage = 4;
        return false;
    }
    case 4:
        if (w == 0xFFFFFFFFu) {
            g80.stage = 8;
        } else {
            g80.part = w;
            g80.sub = 0;
            g80.stage = 5;
        }
        return false;
    case 5: case 6: case 7:
        S[g80.part + g80.obj + 16u * g80.sub] = w;
        g80.sub++;
        if (g80.stage == 7) {
            /* Not Gems: the i960 reads a word back per part (0x8ACA8), and
             * the firmware answers part + 0x20. Gems' interpreter never
             * stalls on an empty FIFO; the board's would. */
            gems_out_w((g80.part & 0xFu) + 0x20u);
            g80.stage = 4;
        } else {
            g80.stage++;
        }
        return false;
    default:
        g80_finish(w);
        gems_out_w(0);
        g80.stage = 0;
        return true;
    }
}

static void gcop_80(void) {
    gcop_zanzou_begin();
    while (!gcop_zanzou_feed(gems_in_w())) {
    }
}
