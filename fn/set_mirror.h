/* set_mirror (i960 0x30E04, Gems 800409AC): mirror the motion record at g5
 * left for right. Angles are negated or taken from 0x8000, float signs
 * flipped, and the left/right word triples swapped (some negated). */

enum { SET_MIRROR_KEEP, SET_MIRROR_NEG, SET_MIRROR_SIGN };

static inline uint32_t set_mirror_op(uint32_t v, int op)
{
    if (op == SET_MIRROR_NEG) return 0u - v;
    if (op == SET_MIRROR_SIGN) return v ^ 0x80000000u;
    return v;
}

/* Swap the words at +a and +b, applying op to both (a written first). */
static inline void set_mirror_swap(uint32_t rec, uint32_t a, uint32_t b, int op)
{
    uint32_t va = set_mirror_op(gems_ld32(rec + a), op);
    uint32_t vb = set_mirror_op(gems_ld32(rec + b), op);
    gems_st32(rec + a, vb);
    gems_st32(rec + b, va);
}

static inline void set_mirror_flip(uint32_t rec, uint32_t a)
{
    gems_st32(rec + a, gems_ld32(rec + a) ^ 0x80000000u);
}

static inline void set_mirror_half_turn(uint32_t rec, uint32_t a)
{
    gems_st16(rec + a, 0x8000u - gems_ld16(rec + a));
}

static uint32_t gfn_set_mirror(int entry)
{
    uint32_t rec = GEMS_G(5);
    gems_st16(rec + 0x34, 0u - (uint32_t)gems_ld16s(rec + 0x34));
    gems_st16(rec + 0x38, 0u - (uint32_t)gems_ld16s(rec + 0x38));
    set_mirror_flip(rec, 0x90);
    set_mirror_half_turn(rec, 0x3C);
    set_mirror_half_turn(rec, 0x44);
    set_mirror_flip(rec, 0x9C);
    set_mirror_flip(rec, 0xA8);
    set_mirror_swap(rec, 0x60, 0x54, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x64, 0x58, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x68, 0x5C, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0xC0, 0xB4, SET_MIRROR_SIGN);
    set_mirror_swap(rec, 0xC4, 0xB8, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0xC8, 0xBC, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0x0C, 0x00, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x10, 0x04, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x14, 0x08, SET_MIRROR_KEEP);
    set_mirror_half_turn(rec, 0x6C);
    set_mirror_half_turn(rec, 0x74);
    set_mirror_swap(rec, 0x84, 0x78, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x88, 0x7C, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x8C, 0x80, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0xE4, 0xD8, SET_MIRROR_SIGN);
    set_mirror_swap(rec, 0xE8, 0xDC, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0xEC, 0xE0, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0x24, 0x18, SET_MIRROR_KEEP);
    set_mirror_swap(rec, 0x28, 0x1C, SET_MIRROR_NEG);
    set_mirror_swap(rec, 0x2C, 0x20, SET_MIRROR_NEG);
    if (entry) gems_i960_ret();
    return 0;
}
