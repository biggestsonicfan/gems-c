/* rand (i960 0x66B0, Gems 800388A8): mixes the four timers at 0xF00000
 * into the seed at 0x500098; g0 = 16 bits of the new seed. */
static uint32_t gfn_rand(int entry)
{
    uint32_t seed = gems_ld32(0x500098);
    seed += gems_ld32(0xF00000) << 4;
    seed += gems_ld32(0xF00004) << 8;
    seed += gems_ld32(0xF00008) << 12;
    seed += gems_ld32(0xF0000C) << 16;
    gems_st32(0x500098, seed);
    GEMS_G(0) = (seed >> 4) & 0xFFFF;
    if (entry) gems_i960_ret();
    return 0;
}
