#pragma once
/* COP op 33 Fn_coli_dist */

static void gcop_33(void) {
    /* An empty handler in Gems: its 12 args are dropped with the input ring
     * and nothing is replied, though the table says 4 reply bytes. */
    for (int i = 0; i < 12; i++) (void)gems_in_w();
}
