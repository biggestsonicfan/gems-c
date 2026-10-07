/* rob_disp_mir (i960 0x64290; Gems 8004FF70): draw a fighter (g7) part by
 * part. Only when bit 7 of *g7 is set: first updates the stage record's
 * float at +0x28C from the fighter's position g7+0x1F4 (the i960 calls
 * 0x64890 for it; Gems inlines it), then for each of the 16 units:
 * op 0x13 on its ball, mark its bit in the mask, push and load its unit
 * matrix (0x37), and draw it (set_obj, model g7+0x40[unit]; unit 1 drops to
 * model 0 under bit 23 of g7+0x70C), with the held item (g7+0x788 / +0x7A0)
 * on its unit, plus the unit's extra (the i960's table at 0x64450: 1 Fang's
 * gun, 2 the paired models at g7+0x2050, 5/8 the reach part, 9 Tails' tails).
 * While g7+0x2068 == 1 unit 0 is the spin ball instead and the others are
 * not drawn. Ends with kosi_nobi_put. The unit mask goes to g7+0x6F2 always. */
#pragma once

/* g7 is read live, as Gems and the i960 both do. */
#define RDM_ROB GEMS_G(7)

static inline void gfn_rob_disp_mir_op13(void)
{
    gems_cop_w(0x09801313u);                    /* op 0x13, args = the command word */
    gems_cop_w(0x09801313u);
    gems_cop_w(0x09801313u);
    (void)gems_cop_r();
}

static void gfn_rob_disp_mir_extra(uint32_t unit)
{
    switch (unit) {
    case 1:
        if (gems_ld8(RDM_ROB + 0x1B1u) == 4u)
            gfn_efc_fang_gun_disp(0);
        break;
    case 2: {
        uint32_t models[2];
        gems_ldn(models, RDM_ROB + 0x2050u, 2);
        if ((gems_ld32(RDM_ROB) & 0x100000u) == 0) {
            GEMS_G(2) = gems_ld32(RDM_ROB + 0x2058u);
            GEMS_G(1) = 0;
            GEMS_G(0) = models[0];
            gfn_set_obj_tpd(0);
            GEMS_G(2) = gems_ld32(RDM_ROB + 0x205Cu);
            GEMS_G(1) = 0;
            GEMS_G(0) = models[1];
            gfn_set_obj_tpd(0);
        } else {
            GEMS_G(1) = 0;
            GEMS_G(0) = models[0];
            gfn_set_obj();
            GEMS_G(1) = 0;
            GEMS_G(0) = models[1];
            gfn_set_obj();
        }
        GEMS_G(1) = 0;                          /* the i960 calls 0x330B8 for this */
        GEMS_G(0) = gems_ld16(RDM_ROB + 0x2A94u);
        gfn_set_obj();
        break;
    }
    case 5:
    case 8:
        gfn_rob_disp_mir_part(0);
        break;
    case 9: {
        uint32_t kind = gems_ld8(RDM_ROB + 0x1B0u);
        if (kind == 1u)
            gfn_tails2_tail_disp(0);
        else if (kind == 27u)
            gfn_tails1_tail_disp(0);
        break;
    }
    default:
        break;
    }
}

static uint32_t gfn_rob_disp_mir(int entry)
{
    uint32_t mask = 0;

    if (gems_ld32(RDM_ROB) & 0x80u) {
        /* i960 0x64890: rec+0x28C = rec+0x290 - rec+0x1C
         *   + k2 * (k0 * (x - rec+0x18) + k1 * (z - rec+0x20)), k = rec+0x280.. */
        gems_ldn(&GEMS_G(0), RDM_ROB + 0x1F4u, 3);
        uint32_t rec = gems_ld32(0x500814u);
        uint32_t org[3], k[3];
        gems_ldn(org, rec + 0x18u, 3);
        gems_ldn(k, rec + 0x280u, 3);
        float tz = gems_u2f(k[1]) * (gems_u2f(GEMS_G(2)) - gems_u2f(org[2]));
        float tx = gems_u2f(k[0]) * (gems_u2f(GEMS_G(0)) - gems_u2f(org[0]));
        float y = gems_u2f(k[2]) * (tx + tz) - gems_u2f(org[1]);
        y = gems_ldf(rec + 0x290u) + y;
        gems_stf(rec + 0x28Cu, y);

        for (uint32_t unit = 0; unit < 16u; unit++) {
            gems_ldn(&GEMS_G(0), RDM_ROB + 0x1F4u + unit * 0xCu, 3);
            gfn_rob_disp_mir_op13();
            mask |= 1u << unit;
            gems_cop_w(0x00800101u);            /* push matrix */
            gems_cop_w(0x1B803737u);            /* mul_unit_mat */
            gems_cop_w(gems_ld8(RDM_ROB + 4u));
            gems_cop_w(unit * 12u);

            if (gems_ld32(RDM_ROB + 0x2068u) == 1u) {
                if (unit == 0) {
                    uint32_t esp = gems_ld8(RDM_ROB + 0x1B1u) % 26u == 7u;
                    GEMS_G(3) = RDM_ROB + 0x2088u;
                    gems_ldn(&GEMS_G(0), GEMS_G(3) + 0x24u, 3);
                    if (esp)
                        gfn_spin_attack_cnt_esp_dsp(0);
                    else
                        gfn_spin_attack_cnt_nml_dsp(0);
                }
            } else {
                uint32_t model = gems_ld32(RDM_ROB + 0x40u + unit * 4u);
                if (unit == 1u && (gems_ld32(RDM_ROB + 0x70Cu) & 0x800000u))
                    model = 0;
                gfn_rob_disp_mir_op13();
                GEMS_G(1) = 0;
                GEMS_G(0) = model;
                gfn_set_obj();

                if (gems_ld16(RDM_ROB + 0x7A2u) != 0) {
                    uint32_t held = RDM_ROB + 0x788u;
                    if (gems_ld32(RDM_ROB) & 0x08000000u)
                        held = RDM_ROB + 0x7A0u;
                    if (gems_ld16(held) == unit) {
                        uint32_t pos[3];
                        gems_cop_w(0x00800101u);        /* push matrix */
                        gems_cop_w(0x03000606u);        /* trans */
                        gems_ldn(pos, held + 4u, 3);
                        gems_cop_wn(pos, 3);
                        gems_cop_w(0x04800909u);        /* ang_y */
                        gems_cop_w((uint32_t)gems_ld16s(held + 0x12u));
                        gems_cop_w(0x04000808u);        /* ang_x */
                        gems_cop_w((uint32_t)gems_ld16s(held + 0x10u));
                        gems_cop_w(0x05000A0Au);        /* ang_z */
                        gems_cop_w((uint32_t)gems_ld16s(held + 0x14u));
                        GEMS_G(1) = 0;
                        GEMS_G(0) = gems_ld16(held + 2u);
                        gfn_set_obj();
                        gems_cop_w(0x01000202u);        /* pop matrix */
                    }
                }
                gfn_rob_disp_mir_extra(unit);
            }
            gems_cop_w(0x01000202u);            /* pop matrix */
        }
        gfn_kosi_nobi_put(0);
    }
    gems_st16(RDM_ROB + 0x6F2u, mask & 0xFFFFu);

    if (entry) gems_i960_ret();
    return 0;
}

#undef RDM_ROB
