/* tobi_disp (i960 0x8A518; Gems 80049E74): draw the 30 flying objects
 * ("tobi", 0x100 bytes each from *(0x5008A4)+0x200): the afterimages first
 * (zanzou_disp), then each live slot (bit 31 set, bit 9 clear) with a
 * nonzero +8 goes through COP 0x13 and its kind's display routine (byte
 * +0x26, the i960 table at 0x8A69C); a live slot with +8 zero is freed.
 *
 * Gems calls its own copies of the display routines (PTR_FUN_80150258):
 *   kinds 0,1,4,5,6,12,13,14  8004B1B0 = i960 0x8A590
 *   kinds 2,3,7,8,9           8004AAF8 = i960 0x8A74C
 *   kind 10                   8004A84C = i960 0x8B644
 *   kind 11                   8004A624 = i960 0x8BA98
 *   kind 15                   8004A434 = i960 0x8BF54
 *   kind 16                   8004A278 = i960 0x8C304
 *   kind 17                   8004A05C = i960 0x8C588
 *   kinds 18-31               8004A054 = i960 0x8A71C, a bare `ret`
 * None of those has a converted equivalent, so a slot whose routine is more
 * than a `ret` hands the rest of the loop to the i960 at its `callx`
 * (0x8A578), with the registers the loop has there. A native call (entry 0)
 * cannot do that and skips the routine. */
#pragma once

static uint32_t gfn_tobi_disp(int entry)
{
    gfn_zanzou_disp(0);
    GEMS_G(3) = 0;
    GEMS_G(6) = gems_ld32(0x5008A4) + 0x200u;
    bool r13_set = false;

    for (uint32_t left = 30;; left--) {
        uint32_t tobi = GEMS_G(6);
        if ((gems_ld32(tobi) & 0x80000000u) && !(gems_ld32(tobi) & 0x200u)) {
            uint32_t life = gems_ld32(tobi + 8);
            if (life == 0) {
                gems_st32(tobi, 0);
            } else {
                gems_cop_w(0x09801313u);         /* COP 0x13, the command word as both args */
                gems_cop_w(0x09801313u);
                gems_cop_w(0x09801313u);
                uint32_t reply = gems_cop_r();
                uint32_t kind = gems_ld8(tobi + 0x26);
                uint32_t routine = gems_ld32(0x8A69C + kind * 4u);
                if (gems_ld32(routine) != 0x0A000000u) {       /* not a bare `ret` */
                    if (!entry) {
                        LOG_WARN("gems: tobi_disp: kind %u's routine 0x%X needs the i960", kind, routine);
                    } else {
                        GEMS_R(3)  = life;
                        GEMS_R(10) = routine;
                        GEMS_R(11) = left;
                        GEMS_R(12) = kind;
                        if (r13_set) GEMS_R(13) = 0x100;
                        GEMS_R(15) = reply;
                        GEMS_AC = (GEMS_AC & ~7u) | 4u;          /* cmpobne 0,r3: 0 < r3 */
                        gems_branch(0x8A578);
                        return 0;
                    }
                }
            }
        }
        r13_set = true;
        GEMS_G(6) += 0x100u;
        if (left <= 1) break;                          /* cmpdeco 1,r11,r11; bl */
    }
    GEMS_AC = (GEMS_AC & ~7u) | 2u;                    /* the last cmpdeco: 1 == 1 */
    if (entry) gems_i960_ret();
    return 0;
}
