/* Gems' COP globals: everything the SHARC decompiles (ann/SHARC/ *.c and
 * helpers/) touch outside the state image, as named statics, and Gems' COP
 * init (0x80022C9C) as gcop_reset().
 *
 * Gems keeps the COP's state in an image of SHARC DM from 0x30000 (its state
 * pointer, sdata 0x801E7C30: byte offset o is DM 0x30000 + o/4) and a 1 KB
 * "inner" block right after it (DM 0x34000). Its two sdata pointers into them
 * are kept here as DM addresses, so `*(float **)(0x801e7c58)` is
 * gems_dmf(gcs_c) and `GCMEM_int(0x801e7c34) + k*4` is gems_dm(gcs_inner + k).
 *
 * Address -> name, for the converters:
 *   sdata (sbss: power-on 0, set by gcop_reset)
 *   0x801E7C30  (state pointer)           DM 0x30000, gems_dm(0x30000 + o/4)
 *   0x801E7C34  inner pointer             gcs_inner      (DM 0x34000)
 *   0x801E7C38  u16 osage record type     gcs_osage_type (4a Fn_osage)
 *   0x801E7C3A..0x801E7C3E u16            gcs_7c3a[3]    (zeroed by init, no readers)
 *   0x801E7C58  "c register" pointer      gcs_c          (DM 0x30400; 1b..20)
 *   0x801E7C60..0x801E7C8C floats         gcs_7c60[(addr - 0x801E7C60) / 4]  (6b temporaries)
 *   0x801E7C90  int                       gcs_7c90       (6b: bufferram index)
 *   0x801E7C94  int                       gcs_7c94       (6b: bufferram index)
 *   0x801E7C98  int                       gcs_7c98       (6b: flag)
 *   0x801E7CA0  int                       gcs_7ca0       (zeroed by 0x80025434, no readers)
 *   0x801E7CA8  int                       gcs_osage_count (4a)
 *   0x801E7CAC  int                       gcs_osage_loop  (4a: loop while non-zero)
 *   sdata2 (constants)
 *   0x801E7A18  float NaN 0x7FFFFFFF      GCS_NAN_BITS / gcs_nanf()
 *   0x801E89F4  100.0f                    GCS_F_100
 *   0x801E8AEC  1.0f                      GCS_F_1
 *   0x801E9468  1.0                       GCS_D_1
 *   0x801E9540  1e-300                    GCS_D_TINY
 *   0x801E9548  1e300                     GCS_D_HUGE
 *   0x801E9560  0.0                       GCS_D_0
 *   0x801E90C0 / 0x801E90C8  asin pS4/pS5 (inside gch_800ab620)
 *   data
 *   DAT_800F9F20  sine table              gc_sin_table (sin_table.h); gcs_sin(a) / gcs_cos(a)
 *   DAT_80146A20/28/30  0.0, 2^32, 2^31   (inside gch_800a2544)
 *   DAT_80146C80  ipio2[66]               gcs_ipio2
 *   DAT_80146D84  npio2_hw - 1 (Ghidra indexes [n-1] from d84; the table starts at d88)  gcs_npio2_hw[32]
 *   DAT_80146E08  init_jk[4]              gcs_init_jk
 *   DAT_80146E18  PIo2[8]                 gcs_PIo2
 *   DAT_80146E58..EB8  kernel_tan T[13]   gcs_tan_T
 *   DAT_8018AF00, DAT_8018B300/B304       Gems' reply ring and its count: gems_out_w() (gch_80022e7c)
 *   PTR_LAB_8014FE48  Fn_osage's record-handler jump table (the 4a converter's)
 */
#ifndef GEMS_SHARC_STATE_H
#define GEMS_SHARC_STATE_H

#include <stdint.h>
#include <string.h>
#include "sin_table.h"

/* ---- sdata ---- */
static uint32_t gcs_inner;          /* 0x801E7C34: DM address of the inner block (0x34000) */
static uint16_t gcs_osage_type;     /* 0x801E7C38 */
static uint16_t gcs_7c3a[3];        /* 0x801E7C3A, 0x801E7C3C, 0x801E7C3E */
static uint32_t gcs_c;              /* 0x801E7C58: DM address of the c register (0x30400) */
static float    gcs_7c60[12];       /* 0x801E7C60..0x801E7C8C */
static int32_t  gcs_7c90;           /* 0x801E7C90 */
static int32_t  gcs_7c94;           /* 0x801E7C94 */
static int32_t  gcs_7c98;           /* 0x801E7C98 */
static int32_t  gcs_7ca0;           /* 0x801E7CA0 */
static int32_t  gcs_osage_count;    /* 0x801E7CA8 */
static int32_t  gcs_osage_loop;     /* 0x801E7CAC */

/* ---- sdata2 ---- */
#define GCS_NAN_BITS 0x7FFFFFFFu    /* 0x801E7A18 */
#define GCS_F_100    100.0f         /* 0x801E89F4 */
#define GCS_F_1      1.0f           /* 0x801E8AEC */
#define GCS_D_1      1.0            /* 0x801E9468 */
#define GCS_D_TINY   1.0e-300       /* 0x801E9540 */
#define GCS_D_HUGE   1.0e+300       /* 0x801E9548 */
#define GCS_D_0      0.0            /* 0x801E9560 */
static inline float gcs_nanf(void) { float f; uint32_t u = GCS_NAN_BITS; memcpy(&f, &u, 4); return f; }

/* ---- the sine table: 0x10000 = 2 pi, only the low 16 bits of an angle count ---- */
#ifdef GEMS_HOST_MATH
/* A host with its own sin/cos of a binary angle (the Dreamcast's FSCA) has
 * GEMS_HOST_SIN / GEMS_HOST_COS and leaves the table out: not the table's words. */
static inline float gcs_sin(uint32_t a) { return GEMS_HOST_SIN(a & 0xFFFFu); }
static inline float gcs_cos(uint32_t a) { return GEMS_HOST_COS(a & 0xFFFFu); }
#else
static inline float gcs_sin(uint32_t a) { float f; memcpy(&f, &gc_sin_table[a & 0xFFFFu], 4); return f; }
static inline float gcs_cos(uint32_t a) { float f; memcpy(&f, &gc_sin_table[(a + 0x4000u) & 0xFFFFu], 4); return f; }
#endif

/* ---- MSL fdlibm tables (data at 0x80146C80..) ---- */
static const int32_t gcs_ipio2[66] = {
    0xA2F983, 0x6E4E44, 0x1529FC, 0x2757D1, 0xF534DD, 0xC0DB62, 0x95993C, 0x439041, 0xFE5163,
    0xABDEBB, 0xC561B7, 0x246E3A, 0x424DD2, 0xE00649, 0x2EEA09, 0xD1921C, 0xFE1DEB, 0x1CB129,
    0xA73EE8, 0x8235F5, 0x2EBB44, 0x84E99C, 0x7026B4, 0x5F7E41, 0x3991D6, 0x398353, 0x39F49C,
    0x845F8B, 0xBDF928, 0x3B1FF8, 0x97FFDE, 0x05980F, 0xEF2F11, 0x8B5A0A, 0x6D1F6D, 0x367ECF,
    0x27CB09, 0xB74F46, 0x3F669E, 0x5FEA2D, 0x7527BA, 0xC7EBE5, 0xF17B3D, 0x0739F7, 0x8A5292,
    0xEA6BFB, 0x5FB11F, 0x8D5D08, 0x560330, 0x46FC7B, 0x6BABF0, 0xCFBC20, 0x9AF436, 0x1DA9E3,
    0x91615E, 0xE61B08, 0x659985, 0x5F14A0, 0x68408D, 0xFFD880, 0x4D7327, 0x310606, 0x1556CA,
    0x73A8C9, 0x60E27B, 0xC08C6B,
};
static const int32_t gcs_npio2_hw[32] = {   /* 0x80146D88 */
    0x3FF921FB, 0x400921FB, 0x4012D97C, 0x401921FB, 0x401F6A7A, 0x4022D97C, 0x4025FDBB, 0x402921FB,
    0x402C463A, 0x402F6A7A, 0x4031475C, 0x4032D97C, 0x40346B9C, 0x4035FDBB, 0x40378FDB, 0x403921FB,
    0x403AB41B, 0x403C463A, 0x403DD85A, 0x403F6A7A, 0x40407E4C, 0x4041475C, 0x4042106C, 0x4042D97C,
    0x4043A28C, 0x40446B9C, 0x404534AC, 0x4045FDBB, 0x4046C6CB, 0x40478FDB, 0x404858EB, 0x404921FB,
};
static const int32_t gcs_init_jk[4] = { 2, 3, 4, 6 };
static const uint64_t gcs_PIo2_bits[8] = {
    0x3FF921FB40000000ull, 0x3E74442D00000000ull, 0x3CF8469880000000ull, 0x3B78CC5160000000ull,
    0x39F01B8380000000ull, 0x387A252040000000ull, 0x36E3822280000000ull, 0x3569F31D00000000ull,
};
static const uint64_t gcs_tan_T_bits[13] = {
    0x3FD5555555555563ull, 0x3FC111111110FE7Aull, 0x3FABA1BA1BB341FEull, 0x3F9664F48406D637ull,
    0x3F8226E3E96E8493ull, 0x3F6D6D22C9560328ull, 0x3F57DBC8FEE08315ull, 0x3F4344D8F2F26501ull,
    0x3F3026F71A8D1068ull, 0x3F147E88A03792A6ull, 0x3F12B80F32F0A7E9ull, 0xBEF375CBDB605373ull,
    0x3EFB2A7074BF7AD4ull,
};
static inline double gcs_d(uint64_t bits) { double d; memcpy(&d, &bits, 8); return d; }
static inline double gcs_PIo2(int i)  { return gcs_d(gcs_PIo2_bits[i]); }
static inline double gcs_tan_T(int i) { return gcs_d(gcs_tan_T_bits[i]); }

/* Gems' COP init (0x80022C9C, with 0x80023228 and 0x80025434): the state
 * image (0x10000 bytes, DM 0x30000..0x33FFF) and the inner block (1 KB,
 * DM 0x34000..0x340FF) are cleared, and the sdata pointers set. The current
 * matrix index and the stack depth come up 0; the i960's first command,
 * Fn_initialize (op 00), sets them to 0x5A0 and 0. The FIFO and reply-ring
 * fields init also clears are the runtime's. */
static void gcop_reset(void) {
    for (uint32_t a = 0x30000u; a < 0x34000u + 256u; a++) *gems_dm(a) = 0;
    gcs_inner = 0x34000u;
    gcs_osage_type = 0;
    memset(gcs_7c3a, 0, sizeof gcs_7c3a);
    gcs_c = 0x30400u;                       /* state + 0x1000 */
    memset(gcs_7c60, 0, sizeof gcs_7c60);
    gcs_7c90 = gcs_7c94 = gcs_7c98 = 0;
    gcs_7ca0 = 0;
    gcs_osage_count = 0;
    gcs_osage_loop = 0;
}

#endif
