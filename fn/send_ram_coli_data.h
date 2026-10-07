/* send_ram_coli_data (i960 0x113C; Gems 800534D8): resend the collision balls. */
#pragma once

static uint32_t gfn_send_ram_coli_data(int entry)
{
    gfn_set_coli_ball_init(0);
    gfn_send_coli_ball_R(0);
    gfn_send_ball_to_unit(0);
    if (entry) gems_i960_ret();
    return 0;
}
