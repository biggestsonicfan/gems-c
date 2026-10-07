/* kosi_nobi_put (i960 0x1A860; Gems 800490A4): when flag 0x800000 of
 * rob+0x70C is up, draw the waist-stretch part (model rob+0x44) between the
 * midpoint of rob+0x26C / rob+0x290 and the point at rob+0x20C: translate to
 * the midpoint, turn towards the end point (COP 0x27 twice for the two
 * angles), scale x by the reply of 0x2C times the per-character factor
 * (0x1AA54[rob+0x1B1]), offset by 0x1A9EC[rob+0x1B1], draw. */
#pragma once

static uint32_t gfn_kosi_nobi_put(int entry)
{
    uint32_t rob = GEMS_G(7);
    if (gems_ld32(rob + 0x70C) & 0x800000) {
        uint32_t a[3], b[3];
        gems_cop_w(0x00800101);                    /* push */
        gems_ldn(a, rob + 0x26C, 3);
        gems_ldn(b, rob + 0x290, 3);
        float half = 0.5f;
        float mx = half * (gems_u2f(a[0]) + gems_u2f(b[0]));
        float my = half * (gems_u2f(a[1]) + gems_u2f(b[1]));
        float mz = half * (gems_u2f(a[2]) + gems_u2f(b[2]));
        gems_cop_w(0x03000606);                    /* trans */
        gems_cop_wf(mx);
        gems_cop_wf(my);
        gems_cop_wf(mz);

        gems_ldn(b, rob + 0x20C, 3);               /* the end point */
        gems_cop_w(0x15802B2B);
        gems_cop_wf(mx);
        gems_cop_w(b[0]);
        gems_cop_wf(mz);
        gems_cop_w(b[2]);
        uint32_t flat = gems_cop_r();
        gems_cop_w(0x16002C2C);
        gems_cop_wf(mx);
        gems_cop_w(b[0]);
        gems_cop_wf(my);
        gems_cop_w(b[1]);
        gems_cop_wf(mz);
        gems_cop_w(b[2]);
        float len = gems_cop_rf();

        float dx = gems_u2f(b[0]) - mx;
        float dy = gems_u2f(b[1]) - my;
        float dz = gems_u2f(b[2]) - mz;
        gems_cop_w(0x13802727);
        gems_cop_wf(dx);
        gems_cop_wf(dz);
        uint32_t yaw = gems_cop_r();
        gems_cop_w(0x04800909);                    /* y_rot */
        gems_cop_w(yaw);
        gems_cop_w(0x13802727);
        gems_cop_w(flat);
        gems_cop_wf(dy);
        uint32_t pitch = 0u - gems_cop_r();
        gems_cop_w(0x05000A0A);                    /* z_rot */
        gems_cop_w(pitch);
        gems_cop_w(0x04000808);                    /* x_rot */
        gems_cop_w(0x8000);

        uint32_t chr = gems_ld8(rob + 0x1B1);
        float k = gems_ldf(chr * 4 + 0x1AA54);
        gems_cop_w(0x03800707);                    /* scale */
        gems_cop_wf(k * len);
        gems_cop_w(0x3F800000);
        gems_cop_w(0x3F800000);

        chr = gems_ld8(rob + 0x1B1);
        uint32_t off = gems_ld32(chr * 4 + 0x1A9EC);
        gems_cop_w(0x03000606);                    /* trans */
        gems_cop_w(off);
        gems_cop_w(0);
        gems_cop_w(0);

        GEMS_G(1) = 0;
        GEMS_G(0) = gems_ld32(rob + 0x44);
        gfn_set_obj();
        gems_cop_w(0x01000202);                    /* pop */
    }
    if (entry) gems_i960_ret();
    return 0;
}
