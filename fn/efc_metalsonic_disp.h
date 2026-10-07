/* efc_metalsonic_disp (i960 0x1AABC, Gems 0x80048F0C): Metal Sonic's jet.
 * Unless bit 29 of rob+0 is set or the body object (rob+0x44) is 0x18D or
 * 0x7E4, draw one of four flame objects picked by bits 1-2 of the frame
 * counter at 0x500020, from 0x1AB24 or (bit 0 set) 0x1AB2C. */
#pragma once

static uint32_t gfn_efc_metalsonic_disp(int entry)
{
    uint32_t rob = GEMS_G(7);
    if (!(gems_ld32(rob) & 0x20000000)) {
        uint32_t body = gems_ld16(rob + 0x44);          /* ldos */
        if (body != 0x18D && body != 0x7E4) {
            uint32_t count = gems_ld32(0x500020);
            uint32_t frame = (count >> 1) & 3;
            uint32_t table = (count & 1) ? 0x1AB2C : 0x1AB24;
            uint32_t obj = gems_ld16(table + frame * 2);  /* ldos */
            GEMS_G(1) = 0;
            GEMS_G(0) = obj;
            gfn_set_obj();
        }
    }
    if (entry) gems_i960_ret();
    return 0;
}
