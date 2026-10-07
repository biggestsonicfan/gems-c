#pragma once
/* COP op 48 Fn_read_ram */

static void gcop_48(void) {
    uint32_t addr = gems_in_w();
    gems_out_w(*gems_dm(addr));
}
