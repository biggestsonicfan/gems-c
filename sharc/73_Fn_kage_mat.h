#pragma once
/* COP op 73 Fn_kage_mat */

/* Builds the three shadow matrices into inner slots 1, 2 and 0 from the light's
 * elevation and heading; DM 0x30800 is the floor height. */
static void gcop_73(void) {
    uint16_t elev = (uint16_t)gems_in_w();
    uint16_t head = (uint16_t)gems_in_w();
    float s, c;
    gch_8001ead8(&s, &c, elev);
    float inv = 1.0f / s;
    float h   = *gems_dmf(0x30800u);
    float tz  = -h * (inv * c);
    uint16_t ry = (head & 0x8000u) ? (uint16_t)(head & 0x7FFFu) : (uint16_t)(head | 0x8000u);
    uint16_t ry_back = (uint16_t)-(int16_t)ry;

    gch_80025350();
    gch_8001e808(ry, *gems_dm(0x3033Fu));
    gch_8001e960(1.0f, 1.0f, inv, *gems_dm(0x3033Fu));
    gch_8001e808(ry_back, *gems_dm(0x3033Fu));
    gch_80024558(1);
    gch_800252e8();

    gch_80025350();
    gch_8001e808(ry, *gems_dm(0x3033Fu));
    gch_8001e9dc(0.0f, h, tz);
    gch_8001e960(1.0f, 1.0f, inv, *gems_dm(0x3033Fu));
    gch_8001e808(ry_back, *gems_dm(0x3033Fu));
    gch_80024558(2);
    gch_800252e8();

    gch_80025350();
    gch_80025214();
    gch_8001e808(ry, *gems_dm(0x3033Fu));
    gch_8001e960(1.0f, 0.0f, 1.0f, *gems_dm(0x3033Fu));
    gch_8001e8b4((uint16_t)(elev - 0x4000u), *gems_dm(0x3033Fu));
    gch_8001e808(ry_back, *gems_dm(0x3033Fu));
    gch_80024558(0);
    gch_800252e8();
}
