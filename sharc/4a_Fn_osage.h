#pragma once
/* COP op 4a Fn_osage */

/* Sway chains. Arg 0 is the bufferram index of a record stream; each record's
 * type word is echoed (u16), and a segment (type 5) also replies its 12-word
 * draw matrix and writes its new point and carry back into the record.
 * State lives at byte offsets of DM 0x30000 (g4a_f(o) = DM 0x30000 + o/4):
 *   3328 stream index         3332 segment record index
 *   3336..3348 floor plane (n, d)   3352..3368 sphere A (c, k, r2)   3372..3388 sphere B
 *   3392/3396 cylinder radius / r2  3400..3452 four side planes (n0, n1, d)
 *   3456 region off (1)       3460 carry gain    3464..3472 chain point
 *   3476/3488 the two ends    3500..3508 offset  3512..3556 segment frame
 *   3560/3608 parent matrices */
static inline float    *g4a_f(uint32_t o) { return gems_dmf(0x30000u + o / 4u); }
static inline uint32_t *g4a_w(uint32_t o) { return gems_dm(0x30000u + o / 4u); }

/* Gems' 0x8001e4ac: out = a (x) b, b's T added (no prototype in helpers_protos.h) */
static void g4a_mat_mul(const float *a, const float *b, float *out) {
    for (int r = 0; r < 4; r++)
        for (int j = 0; j < 3; j++) {
            float v = fmaf(b[6 + j], a[3 * r + 2], fmaf(b[j], a[3 * r], b[3 + j] * a[3 * r + 1]));
            out[3 * r + j] = (r == 3) ? b[9 + j] + v : v;
        }
}

/* Push p out of the sphere at byte offset o (centre, scale, radius squared). */
static void g4a_sphere(float *p, uint32_t o) {
    float dy = p[1] - *g4a_f(o + 4u);
    float dx = p[0] - *g4a_f(o);
    float dz = p[2] - *g4a_f(o + 8u);
    float s = fmaf(dz, dz, fmaf(dx, dx, dy * dy));
    if (!(*g4a_f(o + 16u) >= s)) return;
    float k = *g4a_f(o + 12u) * gch_8001e174(s);
    dx = dx * k;
    dy = dy * k;
    dz = dz * k;
    p[0] = *g4a_f(o) + dx;
    p[1] = *g4a_f(o + 4u) + dy;
    p[2] = *g4a_f(o + 8u) + dz;
}

/* Push p (x, y) onto the inside of the side plane at byte offset o. */
static void g4a_plane(float *p, uint32_t o) {
    float x = p[0];
    float n0 = *g4a_f(o);
    float t = *g4a_f(o + 8u) - fmaf(n0, x, *g4a_f(o + 4u) * p[1]);
    if (!(t >= 0.0f)) return;
    p[0] = fmaf(n0, t, x);
    p[1] = fmaf(*g4a_f(o + 4u), t, p[1]);
}

/* Gems' 0x8002567c: keep a point below the floor plane inside the body regions. */
static void g4a_region(float *p) {
    if (*g4a_w(3456u) == 1u) return;
    float y = p[1];
    if (!(y >= 0.0f)) {
        float x = p[0];
        if (!(x >= 0.0f)) {
            if (y <= *g4a_f(3428u)) g4a_plane(p, 3444u);
            else                    g4a_plane(p, 3412u);
        } else {
            if (!(y <= *g4a_f(3424u))) g4a_plane(p, 3400u);
            else                       g4a_plane(p, 3432u);
        }
        return;
    }
    if (!(y <= *g4a_f(3392u))) {
        /* Gems' 0x8002545c */
        g4a_sphere(p, 3352u);
        g4a_sphere(p, 3372u);
        return;
    }
    float s = fmaf(y, y, p[0] * p[0]);
    if (*g4a_f(3396u) >= s) {
        float k = *g4a_f(3392u) * gch_8001e174(s);
        p[0] = p[0] * k;
        p[1] = p[1] * k;
    }
    g4a_sphere(p, 3372u);
}

/* Gems' 0x80025d90: above the floor plane, project onto it; else the regions. */
static void g4a_constrain(float *p) {
    float n0 = *g4a_f(3336u), n1 = *g4a_f(3340u), n2 = *g4a_f(3344u);
    float t = *g4a_f(3348u) - fmaf(n2, p[2], fmaf(n0, p[0], n1 * p[1]));
    if (t >= 0.0f) {
        p[0] = fmaf(n0, t, p[0]);
        p[1] = fmaf(*g4a_f(3340u), t, p[1]);
        p[2] = fmaf(*g4a_f(3344u), t, p[2]);
    } else {
        g4a_region(p);
    }
}

/* Copy n bufferram words from the record into DM, advancing the record index. */
static void g4a_load(uint32_t dst, uint32_t n) {
    for (uint32_t k = 0; k < n; k++) {
        gems_dm(0x30000u + dst / 4u)[k] = gems_bram_rd((uint32_t)gcs_osage_count);
        gcs_osage_count++;
    }
}

/* One end of the segment: M's T + M (a, b, z) */
static void g4a_end(uint32_t rec, uint32_t m_off, uint32_t dst) {
    float a = gems_bram_rdf(rec);
    float b = gems_bram_rdf(rec + 1u);
    float z = gems_bram_rdf(rec + 2u);
    const float *M = g4a_f(m_off);
    float r0 = M[9]  + fmaf(z, M[6], fmaf(a, M[0], b * M[3]));
    float r1 = M[10] + fmaf(z, M[7], fmaf(a, M[1], b * M[4]));
    float r2 = M[11] + fmaf(z, M[8], fmaf(a, M[2], b * M[5]));
    float *d = g4a_f(dst);
    d[0] = r0;
    d[1] = r1;
    d[2] = r2;
}

/* Gems' 0x8002590c: one segment */
static void g4a_segment(void) {
    uint32_t *idx = g4a_w(3328u);
    *idx += 12u;
    *g4a_w(3332u) = (uint32_t)gcs_osage_count;
    uint32_t rec = (uint32_t)gcs_osage_count;
    *g4a_w(3500u) = gems_bram_rd(rec + 8u);
    *g4a_w(3504u) = gems_bram_rd(rec + 9u);
    *g4a_w(3508u) = gems_bram_rd(rec + 10u);
    *g4a_w(3548u) = *g4a_w(3464u);
    *g4a_w(3552u) = *g4a_w(3468u);
    *g4a_w(3556u) = *g4a_w(3472u);

    g4a_end(rec, 3560u, 3476u);
    g4a_end(rec + 3u, 3608u, 3488u);

    float v[3];
    for (uint32_t i = 0; i < 3; i++) v[i] = *g4a_f(3476u + 4u * i) + *g4a_f(3488u + 4u * i);
    for (uint32_t i = 0; i < 3; i++) v[i] = v[i] + *g4a_f(3500u + 4u * i);
    g4a_constrain(v);
    for (uint32_t i = 0; i < 3; i++) v[i] = v[i] - *g4a_f(3464u + 4u * i);

    float k = gch_8001d9e0(v[0], v[1], v[2]);
    float x = v[0] * k, y = v[1] * k, z = v[2] * k;
    *g4a_f(3524u) = x;
    *g4a_f(3528u) = y;
    *g4a_f(3532u) = z;
    float h = 1.0f - z * z;
    float r = gch_8001e174(h);
    *g4a_f(3544u) = h * r;
    *g4a_f(3512u) = r * y;
    *g4a_f(3540u) = -(*g4a_f(3512u) * z);
    *g4a_f(3516u) = -(r * x);
    *g4a_f(3536u) = *g4a_f(3516u) * z;
    *g4a_f(3520u) = 0.0f;

    float out[12];
    g4a_mat_mul(g4a_f(3512u), gch_cur(), out);
    for (int i = 0; i < 12; i++) gems_out_w(gems_f2u(out[i]));

    gcs_osage_count = (int32_t)*g4a_w(3332u);
    rec = (uint32_t)gcs_osage_count;
    float len = gems_bram_rdf(rec + 7u);
    float p[3];
    p[0] = x * len;
    p[1] = y * len;
    p[2] = z * len;
    for (uint32_t i = 0; i < 3; i++) p[i] = p[i] + *g4a_f(3464u + 4u * i);
    float gain = *g4a_f(3460u);
    gems_bram_wrf(rec, p[0]);
    gems_bram_wrf(rec + 1u, p[1]);
    gems_bram_wrf(rec + 2u, p[2]);
    for (uint32_t i = 0; i < 3; i++)
        gems_bram_wrf(rec + 3u + i, (p[i] - *g4a_f(3476u + 4u * i)) * gain);
    *g4a_f(3464u) = p[0];
    *g4a_f(3468u) = p[1];
    *g4a_f(3472u) = p[2];
}

static void gcop_4a(void) {
    uint32_t *idx = g4a_w(3328u);
    *idx = gems_in_w();
    gcs_osage_loop = 1;
    while (gcs_osage_loop != 0) {
        gcs_osage_count = (int32_t)*idx;
        uint32_t t = gems_bram_rd((uint32_t)gcs_osage_count);
        gcs_osage_type = (uint16_t)t;
        gcs_osage_count++;
        gems_out_w((uint16_t)t);
        switch (gcs_osage_type) {
        case 0:  /* end */
            gcs_osage_loop = 0;
            break;
        case 1:  /* chain root: current matrix and the two parent matrices */
            *idx += 37u;
            g4a_load(*gems_dm(0x3033Fu) * 4u, 12u);
            g4a_load(3560u, 12u);
            g4a_load(3608u, 12u);
            break;
        case 2:  /* collision shape */
            *idx += 31u;
            g4a_load(3336u, 30u);
            break;
        case 3:  /* region switch */
            *idx += 3u;
            g4a_load(3456u, 2u);
            break;
        case 4:  /* chain start point */
            *idx += 4u;
            g4a_load(3464u, 3u);
            break;
        case 5:
            g4a_segment();
            break;
        default:
            /* Gems indexes its 6-entry jump table past the end; no valid stream has these. */
            gcs_osage_loop = 0;
            break;
        }
    }
}
