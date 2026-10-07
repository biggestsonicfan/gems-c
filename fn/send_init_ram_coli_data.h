/* send_init_ram_coli_data (i960 0x1118; Gems 80053528): build the fighter's
 * (g7) collision-ball tables, then send them. g6 picks the table: 0x1008 when
 * rob+4 is 1, else 0xFE8. */
#pragma once

static uint32_t gfn_send_init_ram_coli_data(int entry)
{
    GEMS_G(6) = gems_ld8(GEMS_G(7) + 4u) == 1u ? 0x1008u : 0xFE8u;
    gfn_rob_coli_ball_table_all_init(0);
    gfn_set_coli_ball_init(0);
    gfn_send_coli_ball_R(0);
    gfn_send_ball_to_unit(0);
    if (entry) gems_i960_ret();
    return 0;
}
