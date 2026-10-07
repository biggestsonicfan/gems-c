/* copy_option_data (i960 0x1CBF8; Gems 800389DC): copy the fighter's (g7)
 * option record into rob+0x784..0x7B7 and rob+0x7F8. The records (0x70 bytes,
 * 0x38 more for the second player's colours when bit 6 of rob+0 is set) are
 * at [[rob+0x190]+0x18], indexed by rob+0xAFC - 1 when that is nonzero. */
#pragma once

static uint32_t gfn_copy_option_data(int entry)
{
    uint32_t rob = GEMS_G(7);
    uint32_t src = gems_ld32(gems_ld32(rob + 0x190u) + 0x18u);
    uint32_t option = gems_ld32(rob + 0xAFCu);
    if (option != 0) src += (option - 1u) * 0x70u;
    if (gems_ld32(rob) & 0x40u) src += 0x38u;

    /* { source offset, destination, width } in the order Gems copies them */
    static const struct { uint8_t from; uint16_t to; uint8_t size; } fields[] = {
        { 0x00, 0x784, 4 }, { 0x04, 0x788, 2 }, { 0x06, 0x78A, 2 },
        { 0x08, 0x78C, 4 }, { 0x0C, 0x790, 4 }, { 0x10, 0x794, 4 },
        { 0x14, 0x798, 2 }, { 0x16, 0x79A, 2 }, { 0x18, 0x79C, 2 },
        { 0x1A, 0x79E, 2 }, { 0x1C, 0x7A0, 2 }, { 0x1E, 0x7A2, 2 },
        { 0x20, 0x7A4, 4 }, { 0x24, 0x7A8, 4 }, { 0x28, 0x7AC, 4 },
        { 0x2C, 0x7B0, 2 }, { 0x2E, 0x7B2, 2 }, { 0x30, 0x7B4, 2 },
        { 0x32, 0x7B6, 2 }, { 0x34, 0x7F8, 2 },
    };
    for (size_t i = 0; i < sizeof fields / sizeof fields[0]; i++) {
        if (fields[i].size == 4)
            gems_st32(rob + fields[i].to, gems_ld32(src + fields[i].from));
        else
            gems_st16(rob + fields[i].to, gems_ld16(src + fields[i].from));
    }
    if (entry) gems_i960_ret();
    return 0;
}
