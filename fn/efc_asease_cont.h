/* efc_asease_cont (i960 0x32E88; Gems 80048D2C): the sweat-drop effect. Only
 * when the fighter's character (g7+0x2A70) is 9 and its motion (g7+0x1A8) is
 * not 0xDD: unit scale, z/y/x turns of 0xC000/0x8000/0xC000, and the model
 * from the 32-frame cycle at 0x34E46. */
#pragma once

static uint32_t gfn_efc_asease_cont(int entry)
{
    uint32_t rob = GEMS_G(7);
    if (gems_ld8(rob + 0x2A70) == 9 && gems_ld16(rob + 0x1A8) != 0xDD) {
        gems_cop_w(0x03800707u);          /* scale 1, 1, 1 */
        gems_cop_w(0x3F800000u);
        gems_cop_w(0x3F800000u);
        gems_cop_w(0x3F800000u);
        gems_cop_w(0x05000A0Au);          /* z_rot */
        gems_cop_w(0xC000u);
        gems_cop_w(0x04800909u);          /* y_rot */
        gems_cop_w(0x8000u);
        gems_cop_w(0x04000808u);          /* x_rot */
        gems_cop_w(0xC000u);
        uint32_t frame = gems_ld32(0x500020) & 0x3F;
        GEMS_G(1) = 0;
        GEMS_G(0) = gems_ld16((frame & ~1u) + 0x34E46u);
        gfn_set_obj();
    }
    if (entry) gems_i960_ret();
    return 0;
}
