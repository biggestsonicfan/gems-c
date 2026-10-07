/* os_set_tsukene (i960 0x680E0; Gems 8004CC88): place a sway chain's root
 * point (osage record g9; +0 its kind, +4 the bone, +8 the point) in the
 * world: g0..g2 = the point through COP 0x29 under the matrix the kind
 * names, kept at g13+0xFC. Kind 1 reuses the kept point. The point goes to
 * g8 and g13+0x108. */
#pragma once

static void os_set_tsukene_xform(void)
{
    gems_cop_w(0x14802929);
    gems_cop_w(GEMS_G(0));
    gems_cop_w(GEMS_G(1));
    gems_cop_w(GEMS_G(2));
    GEMS_G(0) = gems_cop_r();
    GEMS_G(1) = gems_cop_r();
    GEMS_G(2) = gems_cop_r();
}

static uint32_t gfn_os_set_tsukene(int entry)
{
    int32_t kind = (int32_t)gems_ld32(GEMS_G(9));
    if (kind == 2) {
        uint32_t bone = gems_ld32(GEMS_G(9) + 4);
        gems_cop_w(0x00800101);                /* push */
        gems_ldn(&GEMS_G(0), GEMS_G(9) + 8, 3);
        gems_cop_w(0x22004444);
        gems_cop_w(1);
        gems_cop_w(0x1B803737);
        gems_cop_w(gems_ld8(GEMS_G(7) + 4));
        gems_cop_w(bone * 0xC);
        os_set_tsukene_xform();
        gems_cop_w(0x21804343);
        gems_cop_w(2);
        gems_cop_w(0x01000202);                /* pop */
        gems_stn(GEMS_G(13) + 0xFC, &GEMS_G(0), 3);
    } else if (kind == 1) {
        gems_ldn(&GEMS_G(0), GEMS_G(13) + 0xFC, 3);
    } else if (kind == 3) {
        gems_cop_w(0x00800101);                /* push */
        gems_ldn(&GEMS_G(0), GEMS_G(9) + 8, 3);
        gems_cop_w(0x22004444);
        gems_cop_w(2);
        os_set_tsukene_xform();
        gems_cop_w(0x01000202);                /* pop */
        gems_stn(GEMS_G(13) + 0xFC, &GEMS_G(0), 3);
    } else {                                   /* 0 (Gems: also <0, >3; the i960's
                                                * bx table has only 0..3) */
        uint32_t bone = gems_ld32(GEMS_G(9) + 4);
        gems_cop_w(0x00800101);                /* push */
        gems_cop_w(0x1B003636);
        gems_cop_w(gems_ld8(GEMS_G(7) + 4));
        gems_cop_w(bone * 0xC);
        gems_ldn(&GEMS_G(0), GEMS_G(9) + 8, 3);
        os_set_tsukene_xform();
        gems_cop_w(0x22004444);
        gems_cop_w(1);
        os_set_tsukene_xform();
        gems_cop_w(0x01000202);                /* pop */
        gems_stn(GEMS_G(13) + 0xFC, &GEMS_G(0), 3);
    }
    gems_stn(GEMS_G(8), &GEMS_G(0), 3);
    gems_stn(GEMS_G(13) + 0x108, &GEMS_G(0), 3);
    if (entry) gems_i960_ret();
    return 0;
}
