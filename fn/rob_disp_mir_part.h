/* rob_disp_mir_part (Gems 0x80050808): the mirror-stage part drawn between
 * a fighter's body and the point it is reaching for (rob+0x230 / +0x254).
 * Not a trap entry: Gems' rob_disp_mir calls it with a pointer to a scratch
 * block in its own frame, which only this function uses, so the scratch is
 * kept in locals here and `entry` is unused. Never pops an i960 frame. */
static uint32_t gfn_rob_disp_mir_part(int entry)
{
    (void)entry;
    uint32_t rob = GEMS_G(7);
    uint32_t flags = gems_ld32(rob + 0x2A3C);
    uint32_t tip[3], base[3];

    if (flags & 0x20)
        gems_ldn(tip, rob + 0x230, 3);
    else if (flags & 0x100)
        gems_ldn(tip, rob + 0x254, 3);
    else
        return 0;

    gems_ldn(base, rob + 0x20C, 3);
    float dx = gems_u2f(tip[0]) - gems_u2f(base[0]);
    float dy = gems_u2f(tip[1]) - gems_u2f(base[1]);
    float dz = gems_u2f(tip[2]) - gems_u2f(base[2]);

    gems_cop_w(0x13802727);             /* yaw = atan2(dx, dz) */
    gems_cop_wf(dx);
    gems_cop_wf(dz);
    uint32_t yaw = gems_cop_r();
    gems_cop_w(0x16802D2D);             /* horizontal length */
    gems_cop_wf(dx);
    gems_cop_wf(dz);
    uint32_t hlen = gems_cop_r();

    uint32_t ndy = gems_f2u(dy) ^ 0x80000000u;
    gems_cop_w(0x13802727);             /* pitch */
    gems_cop_w(hlen);
    gems_cop_w(ndy);
    uint32_t pitch = gems_cop_r();
    gems_cop_w(0x16802D2D);             /* full length */
    gems_cop_w(ndy);
    gems_cop_w(hlen);
    float len = gems_cop_rf() / 0.45f;  /* fdivs by 0x3EE66666 */

    const uint32_t one = 0x3F800000u, neg_one = 0xBF800000u;
    gems_cop_w(0x00800101);             /* push */
    gems_cop_w(0x02000404);             /* load_matrix from rob+0x2A40 */
    for (uint32_t i = 0; i < 12; i++)
        gems_cop_w(gems_ld32(rob + 0x2A40 + i * 4));
    gems_cop_w(0x03800707);             /* scale 1, -1, 1 */
    gems_cop_w(one);
    gems_cop_w(neg_one);
    gems_cop_w(one);
    gems_cop_w(0x03000606);             /* translate to the base */
    gems_cop_wn(base, 3);
    gems_cop_w(0x04800909);
    gems_cop_w(yaw);
    gems_cop_w(0x05000A0A);
    gems_cop_w(pitch);
    gems_cop_w(0x03800707);             /* stretch to the length */
    gems_cop_wf(len);
    gems_cop_w(one);
    gems_cop_w(one);
    GEMS_G(1) = 0;
    GEMS_G(0) = 0x4A5;
    gfn_set_obj();
    gems_cop_w(0x01000202);             /* pop */
    return 0;
}
