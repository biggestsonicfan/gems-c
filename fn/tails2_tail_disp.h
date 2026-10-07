/* tails2_tail_disp (i960 0x1AB48; Gems 800474C8): Tails' second tail. */
#pragma once

static void gfn_tails2_tail_disp(uint32_t entry)
{
    gfn_tails_tail_disp((int32_t)entry, 0x1AF34u, 0x1AE34);
}
