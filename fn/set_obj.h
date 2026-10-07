/* set_obj (i960 0x4E88; Gems 80030028): draw one model, the matrix the COP holds.
 *
 * Gems hands the model to the GameCube renderer (its own model table,
 * FUN_80037840 / GCMEM 0x801E7CC8, and the GX FIFO). This is the i960's
 * set_obj instead, write for write: the polygon budget (0x501018 against
 * 0x50101C), the model-table check, COP 0x13, the object count (0x501010),
 * then COP 0x78 (Fn_set_obj) with the display-list pointer the GEO RAM hands
 * out (0x2008(g10), moved on 0x48 at 0x1008(g10)), g1 (0), the model's
 * table entry and the two counters, whose replies replace them.
 *
 * The gso_* helpers are the code set_obj shares with set_obj_tpd, _thd and
 * _fifo (0x4F94-0x504C, 0x50E4, 0x51B4). They work on r[], the i960's local
 * registers of the function they stand for. */
#pragma once

#define GSO_SET_CC(cc) (GEMS_AC = (GEMS_AC & ~7u) | (uint32_t)(cc))

#define GSO_MODELS_A  0x020E0000u   /* count, then 16-byte entries from +4 */
#define GSO_MODELS_B  0x0647F000u   /* the same for model numbers with bit 15 */

/* A trapped function's locals as the i960 has them; a native call (entry 0)
 * leaves its caller's frame alone and works on zeros. */
static void gso_frame_load(int entry, uint32_t *r)
{
    for (int i = 0; i < 16; i++) r[i] = entry ? GEMS_R(i) : 0;
}

/* `ld 0x501018,r4; ld 0x50101c,r5; cmpobg r5,r4,<ret>`: false when the frame's
 * polygon count is already past the budget. */
static bool gso_budget(uint32_t *r)
{
    r[4] = gems_ld32(0x501018);
    r[5] = gems_ld32(0x50101C);
    if (r[5] > r[4]) { GSO_SET_CC(1); return false; }
    return true;
}

/* `lda (g0),r15 ... cmpobe 0,r14,0x4cb4`: does the model exist? On false the
 * condition code is the failing compare's. */
static bool gso_model_ok(uint32_t model, uint32_t *r)
{
    uint32_t base = GSO_MODELS_A;
    r[15] = model;
    if (model & 0x8000u) { r[15] &= ~0x8000u; base = GSO_MODELS_B; }
    r[14] = gems_ld32(base);
    if (r[15] > r[14]) { GSO_SET_CC(1); return false; }
    r[14] = gems_ld32(base + 4u + r[15] * 16u);
    if (r[14] == 0) { GSO_SET_CC(2); return false; }
    GSO_SET_CC(4);
    return true;
}

/* 0x4FD4 / 0x50E4 / 0x51B4: count the object, point g0 at its model-table
 * entry and load the entry into r8-r11. */
static void gso_take_entry(uint32_t *r)
{
    r[4] = gems_ld32(0x501010) + 1u;
    gems_st32(0x501010, r[4]);
    uint32_t model = GEMS_G(0);
    GSO_SET_CC((model & 0x8000u) ? 2 : 0);                          /* bbs 15,g0 */
    if (model & 0x8000u) GEMS_G(0) = GSO_MODELS_B + 4u + (model & ~0x8000u) * 16u;
    else                 GEMS_G(0) = GSO_MODELS_A + 4u + model * 16u;
    gems_ldn(&r[8], GEMS_G(0), 4);
}

/* A GEO command word: g14 parked at 0x50EFFC while it carries the word. */
static void gso_geo_cmd(uint32_t off, uint32_t word)
{
    gems_st32(0x50EFFC, GEMS_G(14));
    gems_st32(GEMS_G(10) + off, word);
    GEMS_G(14) = gems_ld32(0x50EFFC);
}

/* 0x5008: add the entry's polygon count (low half of its last word) to
 * 0x50101C and send the entry to the GEO as object command 0x101, its last
 * word -1. */
static void gso_send_entry(uint32_t *r)
{
    r[3] = r[11] >> 16;
    r[4] = 0xFFFFu;
    r[11] &= r[4];
    r[4] = gems_ld32(0x50101C) + r[11];
    gems_st32(0x50101C, r[4]);
    r[11] = 0xFFFFFFFFu;
    gso_geo_cmd(0x10, 0x101);
    gems_stn(GEMS_G(10) + GEMS_G(12), &r[8], 4);
}

/* 0x5064-0x50A0 (set_obj_tpd / _thd): GEO command 0xB0B, then COP 0x34
 * (Fn_mov_matrix) lays the current matrix into the display list where the
 * GEO RAM's pointer is, and the pointer moves on 0x30. */
static void gso_mov_matrix(uint32_t *r)
{
    gso_geo_cmd(0xB0, 0xB0B);
    r[15] = 0x1A003434u;
    gems_cop_w(r[15]);
    r[15] = gems_ld32(GEMS_G(10) + 0x2008);
    r[14] = r[15] + 0x30u;
    gems_st32(GEMS_G(10) + 0x1008, r[14]);
    gems_cop_w(r[15]);
    r[15] = gems_cop_r();
}

/* A model the table does not have: the ROM goes to its "max poly / err poly"
 * screen (0x4CB4) and waits for buttons. A trapped call lets the i960 do that
 * from `target` with the registers it would have; a native call cannot. */
static uint32_t gso_bad_model(int entry, const uint32_t *r, uint32_t target, const char *name)
{
    if (!entry) {
        LOG_WARN("gems: %s: model 0x%X is not in the table (the ROM stops on its err poly screen)",
                 name, GEMS_G(0));
        return 0;
    }
    for (int i = 3; i < 16; i++) GEMS_R(i) = r[i];
    gems_branch(target);
    return 0;
}

static uint32_t gfn_set_obj(void)
{
    uint32_t r[16] = {0};

    GEMS_G(1) = 0;
    if (!gso_budget(r)) return 8;
    if (!gso_model_ok(GEMS_G(0), r)) {
        gso_bad_model(0, r, 0x4CB4, "set_obj");
        return 8;
    }

    r[15] = 0x09801313u;                 /* COP 0x13, the command word as both args */
    gems_cop_w(r[15]);
    gems_cop_w(r[15]);
    gems_cop_w(r[15]);
    r[15] = gems_cop_r();
    gems_st32(0x50FFFC, GEMS_G(0));      /* the last model drawn, for the err screen */

    gso_take_entry(r);

    r[15] = 0x3C007878u;                 /* COP 0x78, Fn_set_obj */
    gems_cop_w(r[15]);
    r[15] = gems_ld32(GEMS_G(10) + 0x2008);
    r[14] = r[15] + 0x48u;
    gems_st32(GEMS_G(10) + 0x1008, r[14]);
    gems_cop_w(r[15]);
    gems_cop_w(GEMS_G(1));
    gems_cop_wn(&r[8], 4);
    gems_cop_w(gems_ld32(0x5010D0));
    gems_cop_w(gems_ld32(0x50101C));
    gems_st32(0x5010D0, gems_cop_r());
    gems_st32(0x50101C, gems_cop_r());
    return 8;
}
