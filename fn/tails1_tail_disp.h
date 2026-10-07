/* tails1_tail_disp (i960 0x1AB34; Gems 8004749C): Tails' first tail. */
#pragma once

static void gfn_tails1_tail_disp(uint32_t entry)
{
    gfn_tails_tail_disp((int32_t)entry, 0x1AF38u, 0x1AEB4);
}
