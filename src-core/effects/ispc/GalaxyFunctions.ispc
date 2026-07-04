/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// ISPC gather kernel for Galaxy. Per-pixel inversion of the spiral: each lane handles one
// pixel, walks its covering arms (one per revolution) plus the head/tail rounded caps in the
// same draw order the scalar scatter uses, and combines them the same way. Handles all four
// combos: blend/non-blend edges x outward/inward.
#define MAX_GALAXY_COLORS 8

struct GalaxyISPCData {
    int   width;
    int   height;
    float pos_x, pos_y;
    float radius1, radius2;
    float width1, width2;
    float revs;
    float start_angle;
    int   reverse_dir;
    int   inward;
    int   blend_edges;
    float head_end_of_tail;
    float tail_end_of_tail;
    float color_length;
    int   num_colors;
    float palR[MAX_GALAXY_COLORS];
    float palG[MAX_GALAXY_COLORS];
    float palB[MAX_GALAXY_COLORS];
};

static inline float chBlend(float c1, float c2, float ratio) {
    return c1 + floor(ratio * (c2 - c1) + 0.5f);
}

static inline void galaxyColor(const uniform GalaxyISPCData * uniform d, float i,
                               float &cr, float &cg, float &cb) {
    float cv = (d->head_end_of_tail - i) / d->color_length;
    int ci = (int)cv;
    float cp = cv - (float)ci;
    if (ci < 0) ci = 0;
    int c2 = min(ci + 1, d->num_colors - 1);
    if (ci < c2) {
        cr = chBlend(d->palR[ci], d->palR[c2], cp);
        cg = chBlend(d->palG[ci], d->palG[c2], cp);
        cb = chBlend(d->palB[ci], d->palB[c2], cp);
    } else {
        cr = d->palR[c2]; cg = d->palG[c2]; cb = d->palB[c2];
    }
}

// combine one contribution in draw order.
static inline void galaxyEmit(uniform int blend, float cr, float cg, float cb, float alpha,
                              float &accR, float &accG, float &accB, bool &any) {
    if (alpha <= 0.0f) return;
    if ((int)cr == 0 && (int)cg == 0 && (int)cb == 0) return;
    if (blend) {
        if (!any) { accR = chBlend(0.0f, cr, alpha); accG = chBlend(0.0f, cg, alpha); accB = chBlend(0.0f, cb, alpha); }
        else      { accR = chBlend(accR, cr, alpha); accG = chBlend(accG, cg, alpha); accB = chBlend(accB, cb, alpha); }
    } else {
        // non-blend overwrite: color scaled by brightness (== scalar hsv.value = full*color_pct2)
        accR = cr * alpha; accG = cg * alpha; accB = cb * alpha;
    }
    any = true;
}

static inline void galaxyEmitArm(const uniform GalaxyISPCData * uniform d, uniform int blend,
                                 float i, float rho, float invRevs,
                                 float &accR, float &accG, float &accB, bool &any) {
    float pct = i * invRevs;
    float rc = d->radius2*pct + d->radius1*(1.0f-pct);
    float hw = (d->width2*pct + d->width1*(1.0f-pct)) * 0.5f;
    if (hw <= 0.0f) return;
    float dd = abs(rho - rc);
    if (dd > hw) return;
    float cr, cg, cb;
    galaxyColor(d, i, cr, cg, cb);
    galaxyEmit(blend, cr, cg, cb, 1.0f - dd/hw, accR, accG, accB, any);
}

static inline void galaxyEmitCap(const uniform GalaxyISPCData * uniform d, uniform int blend,
                                 float iEnd, float cd_deg, float rho, float invRevs,
                                 float &accR, float &accG, float &accB, bool &any) {
    float pct = iEnd * invRevs;
    float rc = d->radius2*pct + d->radius1*(1.0f-pct);
    float hw = (d->width2*pct + d->width1*(1.0f-pct)) * 0.5f;
    if (rc < hw || hw <= 0.0f) return;
    float cd = 2.0f*PI*rc*cd_deg/360.0f;
    if (cd >= hw) return;
    float cw = sqrt(hw*hw - cd*cd);
    if (cw <= 0.0f) return;
    float dr = abs(rho - rc);
    if (dr > cw) return;
    float cr, cg, cb;
    galaxyColor(d, iEnd, cr, cg, cb);
    galaxyEmit(blend, cr, cg, cb, (1.0f - dr/cw) * (1.0f - cd/hw), accR, accG, accB, any);
}

export void GalaxyEffectISPC(const uniform GalaxyISPCData * uniform d,
                             uniform int startIdx, uniform int endIdx,
                             uniform uint8<4> * uniform result) {
    uniform int   width  = d->width;
    uniform float px     = d->pos_x,  py = d->pos_y;
    uniform float invRevs = 1.0f / d->revs;
    uniform float sa     = d->start_angle;
    uniform int   rev    = d->reverse_dir;
    uniform int   inward = d->inward;
    uniform int   blend  = d->blend_edges;
    uniform float i_lo   = max(0.0f, d->tail_end_of_tail);
    uniform float i_hi   = min(d->head_end_of_tail, d->revs);
    uniform int   maxRev = (int)((i_hi - i_lo) / 360.0f) + 2;

    if (i_hi <= i_lo) return;

    foreach (index = startIdx ... endIdx) {
        int x = index % width;
        int y = index / width;
        float dx = (float)x - px;
        float dy = (float)y - py;
        float rho = sqrt(dx*dx + dy*dy);
        float theta = atan2(dx, dy) * 180.0f / PI;

        float base = rev ? (-theta - sa) : (theta - sa);
        base = base - floor(base / 360.0f) * 360.0f;
        int   k0    = (int)ceil((i_lo - base) / 360.0f);
        float icapT = base + 360.0f * (ceil((i_lo - base)/360.0f) - 1.0f);
        float icapH = base + 360.0f * (floor((i_hi - base)/360.0f) + 1.0f);

        float accR = 0.0f, accG = 0.0f, accB = 0.0f;
        bool any = false;

        if (!inward) {                     // tail cap, arms tail->head, head cap
            if (icapT < i_lo) galaxyEmitCap(d, blend, i_lo, i_lo - icapT, rho, invRevs, accR, accG, accB, any);
            for (uniform int m = 0; m < maxRev; m++) {
                float i = base + 360.0f * (float)(k0 + m);
                if (i >= i_lo && i <= i_hi) galaxyEmitArm(d, blend, i, rho, invRevs, accR, accG, accB, any);
            }
            if (icapH > i_hi) galaxyEmitCap(d, blend, i_hi, icapH - i_hi, rho, invRevs, accR, accG, accB, any);
        } else {                           // head cap, arms head->tail, tail cap
            if (icapH > i_hi) galaxyEmitCap(d, blend, i_hi, icapH - i_hi, rho, invRevs, accR, accG, accB, any);
            for (uniform int m = maxRev - 1; m >= 0; m--) {
                float i = base + 360.0f * (float)(k0 + m);
                if (i >= i_lo && i <= i_hi) galaxyEmitArm(d, blend, i, rho, invRevs, accR, accG, accB, any);
            }
            if (icapT < i_lo) galaxyEmitCap(d, blend, i_lo, i_lo - icapT, rho, invRevs, accR, accG, accB, any);
        }

        if (any) {
            uint8<4> out;
            out[0] = (uint8)min(255.0f, accR);
            out[1] = (uint8)min(255.0f, accG);
            out[2] = (uint8)min(255.0f, accB);
            out[3] = 255;
            result[index] = out;
        }
    }
}
