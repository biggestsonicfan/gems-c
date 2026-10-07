/* move_tpd_send (i960 0x7FC1C; Gems 80045420): send the frame's replaced
 * texture points (eyes, mouths: move_tpd_req's queue at 0x5019B0) to the GEO.
 * With a queue: GEO command 0x404 at g10+0x40, then the destination
 * (0x805000) and the word count down the FIFO (g10+g12), then each queued
 * record (16 bytes from 0x5019BC; +4 its point list, 0 ends it) by its type
 * (+0, the i960's table at 0x7FCAC), then GEO 0x1010 at g10+0x100 and r3 down
 * the FIFO, and the queue is emptied (the i960's 0x7FB78).
 * g14 is parked at 0x50EFFC around each command, as the i960 does. */
#pragma once

/* Type 0 (i960 0x7FCB4): a count (halfword), then (x, y, x, y) signed
 * halfword quads moved by the record's offset (+8 x, +0xA y); an odd count
 * ends with one (x, y) pair. */
static void move_tpd_send_moved(void)
{
    uint32_t list = gems_ld32(GEMS_G(1) + 4);
    uint32_t dx = gems_ld16(GEMS_G(1) + 8);
    uint32_t dy = gems_ld16(GEMS_G(1) + 10);
    uint32_t count = gems_ld16(list);
    uint32_t pairs = count >> 1;
    uint32_t q[4];
    list += 4;
    for (;;) {
        q[0] = (uint32_t)gems_ld16s(list + 0) + dx;
        q[1] = (uint32_t)gems_ld16s(list + 2) + dy;
        q[2] = (uint32_t)gems_ld16s(list + 4) + dx;
        q[3] = (uint32_t)gems_ld16s(list + 6) + dy;
        gems_stn(GEMS_G(10) + GEMS_G(12), q, 4);
        list += 8;
        uint32_t n = pairs--;
        if (n <= 1) break;
    }
    if (count & 1) {
        q[0] = (uint32_t)gems_ld16s(list + 0) + dx;
        q[1] = (uint32_t)gems_ld16s(list + 2) + dy;
        gems_stn(GEMS_G(10) + GEMS_G(12), q, 2);
    }
}

/* Type 1 (i960 0x7FD1C): a count (halfword), then 8-byte entries sent as
 * (+0, the record's +8, +4, +6), zero-extended. */
static void move_tpd_send_plain(void)
{
    uint32_t list = gems_ld32(GEMS_G(1) + 4);
    uint32_t page = gems_ld16(GEMS_G(1) + 8);
    uint32_t count = gems_ld16(list);
    uint32_t q[4];
    list += 4;
    for (;;) {
        q[0] = gems_ld16(list + 0);
        q[1] = page;
        q[2] = gems_ld16(list + 4);
        q[3] = gems_ld16(list + 6);
        gems_stn(GEMS_G(10) + GEMS_G(12), q, 4);
        list += 8;
        uint32_t n = count--;
        if (n <= 1) break;
    }
}

static uint32_t gfn_move_tpd_send(int entry)
{
    uint32_t left = gems_ld32(0x5019B0);
    if (left != 0) {
        gems_st32(0x50EFFC, GEMS_G(14));
        GEMS_G(14) = 0x404;
        gems_st32(GEMS_G(10) + 0x40, GEMS_G(14));
        GEMS_G(14) = gems_ld32(0x50EFFC);
        gems_st32(GEMS_G(10) + GEMS_G(12), 0x805000);
        gems_st32(GEMS_G(10) + GEMS_G(12), gems_ld32(0x5019B4));
        GEMS_G(1) = 0x5019BC;
        for (;;) {
            if (gems_ld32(GEMS_G(1) + 4) == 0) break;
            GEMS_R(3) = gems_ld16(GEMS_G(1));
            if (GEMS_R(3) == 0) move_tpd_send_moved();
            else move_tpd_send_plain();   /* Gems' table has only types 0 and 1 */
            GEMS_G(1) += 0x10;
            uint32_t n = left--;
            if (n <= 1) break;
        }
        gems_st32(0x50EFFC, GEMS_G(14));
        GEMS_G(14) = 0x1010;
        gems_st32(GEMS_G(10) + 0x100, GEMS_G(14));
        GEMS_G(14) = gems_ld32(0x50EFFC);
        gems_st32(GEMS_G(10) + GEMS_G(12), GEMS_R(3));
        /* the i960's 0x7FB78: empty the queue */
        static const uint32_t zero[2] = { 0, 0 };
        gems_st32(0x5019B8, 0x5019BC);
        gems_st32(0x5019B0, 0);
        gems_st32(0x5019B4, 0);
        gems_stn(0x5019BC, zero, 2);
    }
    if (entry) gems_i960_ret();
    return 0;
}
