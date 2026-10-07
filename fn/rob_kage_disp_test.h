/* rob_kage_disp_test (i960 0x63C40, trap 0x74): fighter g7's shadow, drawn
 * in GEO mode 2 under the kage matrix (0x22004444). g4 = mask of units to
 * draw, g5 = the kage argument. */

/* g4 bit test as Gems does it: PowerPC slw (shift's low 6 bits, 32-63 = 0). */
static inline int gfn_rob_kage_disp_test__bit(uint32_t n)
{
    n &= 63;
    return n < 32 && (GEMS_G(4) & (1u << n)) != 0;
}

static void gfn_rob_kage_disp_test__sync(void)
{
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    gems_cop_w(0x09801313);
    (void)gems_cop_r();
}

static uint32_t gfn_rob_kage_disp_test(int entry)
{
    uint32_t rob = GEMS_G(7);

    GEMS_G(0) = 2;
    gfn_set_mmode(0);
    gems_cop_w(0x00800101);
    gems_cop_w(0x22004444);
    gems_cop_w(0);

    /* 16 parts, 0x18 bytes each: unit, object, kage arg, 3 words -> g0-g2 */
    uint32_t part = gems_ld32(0x64988 + gems_ld8(rob + 0x1B0) * 4);
    for (int i = 0; i < 16; i++, part += 0x18) {
        uint32_t unit = gems_ld32(part);
        uint32_t arg = gems_ld32(part + 8);
        gems_ldn(&GEMS_G(0), part + 0xC, 3);
        if (unit == 9) {
            uint32_t st = gems_ld32(rob + 0x1A4);
            if ((st & 0x4000) && (st & 0x10000))
                continue;
        }
        if (!gfn_rob_kage_disp_test__bit(unit))
            continue;
        gems_cop_w(0x3A007474);
        gems_cop_w(gems_ld8(rob + 4));
        gems_cop_w(unit * 12);
        gems_cop_w(arg);
        gems_cop_w(GEMS_G(0));
        gems_cop_w(GEMS_G(1));
        gems_cop_w(GEMS_G(2));
        gems_cop_w(GEMS_G(5));
        gfn_rob_kage_disp_test__sync();
        uint32_t obj = gems_ld32(part + 4);
        if (gems_ld32(rob + 0x2068) == 1) {
            obj = 0;
            if (unit == 0) {
                uint32_t s[3];
                gems_ldn(s, rob + 0x2074, 3);
                gems_cop_w(0x03800707);
                gems_cop_wn(s, 3);
                obj = 0x195;
            }
        }
        GEMS_G(1) = 0;
        GEMS_G(0) = obj;
        gfn_set_obj();
        gems_cop_w(0x01000202);
    }

    gems_cop_w(0x01000202);
    gems_cop_w(0x00800101);
    gems_cop_w(0x22004444);
    gems_cop_w(0);

    /* the held item's shadow; `part` is now the table's trailer */
    uint32_t obj = gems_ld16(part);                          /* ldos */
    if (obj != 0 && gems_ld16(rob + 0x78A) != 0) {
        if (!(gems_ld32(rob) & 0x8000000)) {
            uint32_t mat = rob + gems_ld32(rob + 0xF00) * 0x34 + 0xF34;
            gfn_rob_kage_disp_test__sync();
            if (gfn_rob_kage_disp_test__bit(gems_ld16(rob + 0x788))) {
                gfn_rob_kage_disp_test__sync();
                gems_cop_w(0x05800B0B);
                for (uint32_t k = 0; k < 4; k++) {
                    uint32_t v[3];
                    gems_ldn(v, mat + k * 0xC, 3);
                    gems_cop_wn(v, 3);
                }
                gfn_rob_kage_disp_test__sync();
                gems_cop_w(0x23004646);
                gems_cop_w(GEMS_G(5));
                gfn_rob_kage_disp_test__sync();
                GEMS_G(1) = 0;
                GEMS_G(0) = obj;
                gfn_set_obj();
            }
        } else {
            obj = gems_ld16(part + 2);
            uint32_t obj_alt = gems_ld16(part + 4);
            if (obj != 0 && gems_ld16(rob + 0x7A2) != 0) {
                uint32_t unit = gems_ld16(rob + 0x7A0);
                if (gfn_rob_kage_disp_test__bit(unit)) {
                    gems_cop_w(0x1B803737);
                    gems_cop_w(gems_ld8(rob + 4));
                    gems_cop_w(unit * 12);
                    gfn_rob_kage_disp_test__sync();
                    gems_cop_w(0x23004646);
                    gems_cop_w(GEMS_G(5));
                    if (gems_ld32(rob) & 0x40)
                        obj = obj_alt;
                    GEMS_G(1) = 0;
                    GEMS_G(0) = obj;
                    gfn_set_obj();
                }
            }
        }
    }

    gems_cop_w(0x01000202);
    GEMS_G(0) = 1;
    gfn_set_mmode(0);
    if (entry)
        gems_i960_ret();
    return 0;
}
