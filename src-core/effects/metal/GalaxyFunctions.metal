/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"

// GPU port of the Galaxy "New Render Method" gather (mirrors GalaxyFunctions.ispc). Each
// thread computes one output pixel by walking its covering spiral arms (one per revolution)
// plus the head/tail rounded caps in draw order and combining them the same way the CPU
// path does. Handles blend/hard edges x outward/inward.

static inline float galChBlend(float c1, float c2, float ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5f);
}

static void galaxyColor(constant MetalGalaxyData &d, float i,
                        thread float &cr, thread float &cg, thread float &cb) {
    float cv = (d.head_end_of_tail - i) / d.color_length;
    int ci = (int)cv;
    float cp = cv - (float)ci;
    if (ci < 0) ci = 0;
    int c2 = min(ci + 1, d.num_colors - 1);
    if (ci < c2) {
        cr = galChBlend(d.palR[ci], d.palR[c2], cp);
        cg = galChBlend(d.palG[ci], d.palG[c2], cp);
        cb = galChBlend(d.palB[ci], d.palB[c2], cp);
    } else {
        cr = d.palR[c2]; cg = d.palG[c2]; cb = d.palB[c2];
    }
}

static void galaxyEmit(int blend, float cr, float cg, float cb, float alpha,
                       thread float &accR, thread float &accG, thread float &accB, thread bool &any) {
    if (alpha <= 0.0f) return;
    if ((int)cr == 0 && (int)cg == 0 && (int)cb == 0) return;
    if (blend) {
        if (!any) { accR = galChBlend(0.0f, cr, alpha); accG = galChBlend(0.0f, cg, alpha); accB = galChBlend(0.0f, cb, alpha); }
        else      { accR = galChBlend(accR, cr, alpha); accG = galChBlend(accG, cg, alpha); accB = galChBlend(accB, cb, alpha); }
    } else {
        accR = cr * alpha; accG = cg * alpha; accB = cb * alpha;
    }
    any = true;
}

static void galaxyEmitArm(constant MetalGalaxyData &d, int blend, float i, float rho, float invRevs,
                          thread float &accR, thread float &accG, thread float &accB, thread bool &any) {
    float pct = i * invRevs;
    float rc = d.radius2*pct + d.radius1*(1.0f-pct);
    float hw = (d.width2*pct + d.width1*(1.0f-pct)) * 0.5f;
    if (hw <= 0.0f) return;
    float dd = fabs(rho - rc);
    if (dd > hw) return;
    float cr, cg, cb;
    galaxyColor(d, i, cr, cg, cb);
    galaxyEmit(blend, cr, cg, cb, 1.0f - dd/hw, accR, accG, accB, any);
}

// Coverage alpha of the arm at parameter i for a pixel at radius rho, or -1 when the arm
// is out of range / doesn't reach the pixel.
static float galaxyArmAlpha(constant MetalGalaxyData &d, float i, float i_lo, float i_hi,
                            float rho, float invRevs) {
    if (i < i_lo || i > i_hi) return -1.0f;
    float pct = i * invRevs;
    float rc = d.radius2*pct + d.radius1*(1.0f-pct);
    float hw = (d.width2*pct + d.width1*(1.0f-pct)) * 0.5f;
    if (hw <= 0.0f) return -1.0f;
    return 1.0f - fabs(rho - rc)/hw;
}

// Through-center spill: the scalar arm loop sweeps r from (rc - hw), which goes negative
// when the width exceeds the radius; negative r plots on the opposite side of the center.
// So the arm half a revolution away (i differs by 180) bleeds across the center onto this
// pixel with alpha = 1 - (rho + rc)/hw. Without this the wide-arm-small-radius look (big
// Start Width, small Start Radius) loses an entire lobe that the old renderer drew.
static float galaxySpillAlpha(constant MetalGalaxyData &d, float i, float i_lo, float i_hi,
                              float rho, float invRevs) {
    if (i < i_lo || i > i_hi) return -1.0f;
    float pct = i * invRevs;
    float rc = d.radius2*pct + d.radius1*(1.0f-pct);
    float hw = (d.width2*pct + d.width1*(1.0f-pct)) * 0.5f;
    if (hw <= 0.0f) return -1.0f;
    return 1.0f - (rho + rc)/hw;
}

static void galaxyEmitAt(constant MetalGalaxyData &d, int blend, float i, float alpha,
                         thread float &accR, thread float &accG, thread float &accB, thread bool &any) {
    float cr, cg, cb;
    galaxyColor(d, i, cr, cg, cb);
    galaxyEmit(blend, cr, cg, cb, alpha, accR, accG, accB, any);
}

static void galaxyEmitCap(constant MetalGalaxyData &d, int blend, float iEnd, float cd_deg, float rho, float invRevs,
                          thread float &accR, thread float &accG, thread float &accB, thread bool &any) {
    float pct = iEnd * invRevs;
    float rc = d.radius2*pct + d.radius1*(1.0f-pct);
    float hw = (d.width2*pct + d.width1*(1.0f-pct)) * 0.5f;
    if (rc < hw || hw <= 0.0f) return;
    float cd = 2.0f*M_PI_F*rc*cd_deg/360.0f;
    if (cd >= hw) return;
    float cw = sqrt(hw*hw - cd*cd);
    if (cw <= 0.0f) return;
    float dr = fabs(rho - rc);
    if (dr > cw) return;
    float cr, cg, cb;
    galaxyColor(d, iEnd, cr, cg, cb);
    galaxyEmit(blend, cr, cg, cb, (1.0f - dr/cw) * (1.0f - cd/hw), accR, accG, accB, any);
}

kernel void GalaxyEffect(constant MetalGalaxyData &d [[buffer(0)]],
                         device uchar4 *result       [[buffer(1)]],
                         uint index                  [[thread_position_in_grid]]) {
    if (index >= d.width * d.height) return;
    uint px = index % d.width;
    uint py = index / d.width;

    float i_lo = max(0.0f, d.tail_end_of_tail);
    float i_hi = min(d.head_end_of_tail, d.revs);
    if (i_hi <= i_lo) { result[index] = uchar4(0, 0, 0, 0); return; }

    float dx = (float)px + 0.5f - d.pos_x;
    float dy = (float)py + 0.5f - d.pos_y;
    float rho = sqrt(dx*dx + dy*dy);
    float theta = atan2(dx, dy) * 180.0f / M_PI_F;

    float base = d.reverse_dir ? (-theta - d.start_angle) : (theta - d.start_angle);
    base = base - floor(base / 360.0f) * 360.0f;
    float invRevs = 1.0f / d.revs;
    int blend = d.blend_edges;

    int   k0    = (int)ceil((i_lo - base) / 360.0f);
    int   maxRev = (int)((i_hi - i_lo) / 360.0f) + 2;
    float icapT = base + 360.0f * (ceil((i_lo - base)/360.0f) - 1.0f);
    float icapH = base + 360.0f * (floor((i_hi - base)/360.0f) + 1.0f);

    float accR = 0.0f, accG = 0.0f, accB = 0.0f;
    bool any = false;

    // Spill arms sit at i +/- 180 from the pixel's own arms, interleaving between them at
    // exactly 180-degree spacing. The scalar renderer's deferred blend-down only preserves
    // a pending write once the sweep has moved 180+ degrees past it, so any contribution
    // whose 180-later neighbor also covers the pixel gets overwritten, never blended.
    // Emit in draw order (tail->head, or head->tail for inward), dropping each
    // contribution whose successor covers the pixel. hw - rc is linear in pct, so its max
    // over the spiral is at an endpoint; when it never goes positive no spill exists
    // anywhere and every arm's successor misses (most effects take that path).
    bool spillOn = max(d.width1*0.5f - d.radius1, d.width2*0.5f - d.radius2) > 0.0f;

    if (!spillOn) {
        if (!d.inward) {                   // tail cap, arms tail->head, head cap
            if (icapT < i_lo) galaxyEmitCap(d, blend, i_lo, i_lo - icapT, rho, invRevs, accR, accG, accB, any);
            for (int m = 0; m < maxRev; m++) {
                float i = base + 360.0f * (float)(k0 + m);
                if (i >= i_lo && i <= i_hi) galaxyEmitArm(d, blend, i, rho, invRevs, accR, accG, accB, any);
            }
            if (icapH > i_hi) galaxyEmitCap(d, blend, i_hi, icapH - i_hi, rho, invRevs, accR, accG, accB, any);
        } else {                           // head cap, arms head->tail, tail cap
            if (icapH > i_hi) galaxyEmitCap(d, blend, i_hi, icapH - i_hi, rho, invRevs, accR, accG, accB, any);
            for (int m = maxRev - 1; m >= 0; m--) {
                float i = base + 360.0f * (float)(k0 + m);
                if (i >= i_lo && i <= i_hi) galaxyEmitArm(d, blend, i, rho, invRevs, accR, accG, accB, any);
            }
            if (icapT < i_lo) galaxyEmitCap(d, blend, i_lo, i_lo - icapT, rho, invRevs, accR, accG, accB, any);
        }
    } else if (!d.inward) {                // tail cap, spill/arm tail->head, head cap
        if (icapT < i_lo) galaxyEmitCap(d, blend, i_lo, i_lo - icapT, rho, invRevs, accR, accG, accB, any);
        float spI = base + 360.0f * (float)(k0 - 1) + 180.0f;
        float spA = galaxySpillAlpha(d, spI, i_lo, i_hi, rho, invRevs);
        for (int m = 0; m < maxRev; m++) {
            float i = base + 360.0f * (float)(k0 + m);
            float aArm = galaxyArmAlpha(d, i, i_lo, i_hi, rho, invRevs);
            if (spA > 0.0f && aArm <= 0.0f) galaxyEmitAt(d, blend, spI, spA, accR, accG, accB, any);
            spI = i + 180.0f;
            spA = galaxySpillAlpha(d, spI, i_lo, i_hi, rho, invRevs);
            if (aArm > 0.0f && spA <= 0.0f) galaxyEmitAt(d, blend, i, aArm, accR, accG, accB, any);
        }
        if (spA > 0.0f) galaxyEmitAt(d, blend, spI, spA, accR, accG, accB, any);
        if (icapH > i_hi) galaxyEmitCap(d, blend, i_hi, icapH - i_hi, rho, invRevs, accR, accG, accB, any);
    } else {                               // head cap, spill/arm head->tail, tail cap
        if (icapH > i_hi) galaxyEmitCap(d, blend, i_hi, icapH - i_hi, rho, invRevs, accR, accG, accB, any);
        float spA = galaxySpillAlpha(d, base + 360.0f * (float)(k0 + maxRev - 1) + 180.0f, i_lo, i_hi, rho, invRevs);
        float spI = 0.0f;
        for (int m = maxRev - 1; m >= 0; m--) {
            float i = base + 360.0f * (float)(k0 + m);
            float aArm = galaxyArmAlpha(d, i, i_lo, i_hi, rho, invRevs);
            if (spA > 0.0f && aArm <= 0.0f) galaxyEmitAt(d, blend, i + 180.0f, spA, accR, accG, accB, any);
            spI = i - 180.0f;
            spA = galaxySpillAlpha(d, spI, i_lo, i_hi, rho, invRevs);
            if (aArm > 0.0f && spA <= 0.0f) galaxyEmitAt(d, blend, i, aArm, accR, accG, accB, any);
        }
        if (spA > 0.0f) galaxyEmitAt(d, blend, spI, spA, accR, accG, accB, any);
        if (icapT < i_lo) galaxyEmitCap(d, blend, i_lo, i_lo - icapT, rho, invRevs, accR, accG, accB, any);
    }

    if (any) {
        result[index] = uchar4((uchar)min(255.0f, accR), (uchar)min(255.0f, accG), (uchar)min(255.0f, accB), 255);
    } else {
        result[index] = uchar4(0, 0, 0, 0);
    }
}
