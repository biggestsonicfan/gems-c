/* ground_disp (i960 0x28448; Gems 8004F010): the stage's ground. Pushes the
 * matrix and applies the stage camera (angles -z, -x, -y from 0x50A020..24,
 * then -T from 0x50A014..1C). Unless the stage record (0x8F3D0 + stage*0x100)
 * has flag 0x2000, or the stage is 9 with flag 0x40 at 0x500498, clips the
 * ground's points (clip_point_check_yoko, g1 = 0x90654) and draws the ground
 * chunks scaled by 1.6 (area_clip, g2 = 0x904D0, g3 = the record's +0x64),
 * running stage 9's rising floor first. Stages 0, 13 and 14 also draw model
 * 0x22B (set_obj_thd, g2 = [0x501A44]) at scale 1.6. Pops the matrix. */
#pragma once

static uint32_t gfn_ground_disp(int entry)
{
    const uint32_t scale = 0x3FCCCCCDu;   /* 1.6f */
    gems_cop_w(0x00800101u);              /* push matrix */
    uint32_t stage = gems_ld8(0x500064u);
    uint32_t stage_flags = gems_ld32(0x8F3D0u + stage * 0x100u);
    gems_cop_w(0x05000A0Au);              /* ang_z */
    gems_cop_w((uint32_t)-gems_ld16s(0x50A024u));
    gems_cop_w(0x04000808u);              /* ang_x */
    gems_cop_w((uint32_t)-gems_ld16s(0x50A020u));
    gems_cop_w(0x04800909u);              /* ang_y */
    gems_cop_w((uint32_t)-gems_ld16s(0x50A022u));
    gems_cop_w(0x03000606u);              /* translate */
    gems_cop_w(gems_ld32(0x50A014u) ^ 0x80000000u);
    gems_cop_w(gems_ld32(0x50A018u) ^ 0x80000000u);
    gems_cop_w(gems_ld32(0x50A01Cu) ^ 0x80000000u);

    if ((stage_flags & 0x2000u) == 0 &&
        (stage != 9u || (gems_ld32(0x500498u) & 0x40u) == 0)) {
        GEMS_G(1) = 0x90654u;
        gfn_clip_point_check_yoko(0);
        GEMS_G(2) = 0x904D0u;
        stage = gems_ld8(0x500064u);
        GEMS_G(3) = stage * 0x100u + 0x8F434u;
        gems_cop_w(0x00800101u);          /* push matrix */
        if (stage == 9u) gfn_stage_pos_up_cnt(0);
        gems_cop_w(0x03800707u);          /* scale */
        gems_cop_w(scale);
        gems_cop_w(scale);
        gems_cop_w(scale);
        gfn_area_clip(0);
        gems_cop_w(0x01000202u);          /* pop matrix */
    }

    stage = gems_ld8(0x500064u);
    if (stage == 14u || stage == 0u || stage == 13u) {
        uint32_t arg = gems_ld32(0x501A44u);
        gems_cop_w(0x00800101u);          /* push matrix */
        gems_cop_w(0x03800707u);          /* scale */
        gems_cop_w(scale);
        gems_cop_w(scale);
        gems_cop_w(scale);
        GEMS_G(2) = arg;
        GEMS_G(1) = 0;
        GEMS_G(0) = 0x22Bu;
        gfn_set_obj_thd(0);
        gems_cop_w(0x01000202u);          /* pop matrix */
    }
    gems_cop_w(0x01000202u);              /* pop matrix */
    if (entry) gems_i960_ret();
    return 0;
}
