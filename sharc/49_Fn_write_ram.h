#pragma once
/* COP op 49 Fn_write_ram */

static void gcop_49(void) {
    uint32_t addr = gems_in_w();
    uint32_t v    = gems_in_w();
    *gems_dm(addr) = v;
}
