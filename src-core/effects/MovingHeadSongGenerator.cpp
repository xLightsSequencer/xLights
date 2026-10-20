#include "MovingHeadSongGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <map>

namespace MovingHeadSong {

namespace {

enum class BarKind { Pool, Chorus, Sweep, Outro };

float Fan(int head, int headCount) {
    if (headCount <= 1) {
        return 0.0f;
    }
    float half = (headCount - 1) / 2.0f;
    return (head - half) / half;
}

std::string Fmt(const char* fmt, double v) {
    char buf[64];
    snprintf(buf, sizeof(buf), fmt, v);
    return buf;
}

std::string DimmerText(const std::vector<float>& pts) {
    std::string s;
    for (size_t i = 0; i < pts.size(); ++i) {
        if (i != 0) {
            s += ",";
        }
        s += Fmt("%g", pts[i]);
    }
    return s;
}

std::string EscapeCommas(const std::string& in) {
    std::string out;
    for (char c : in) {
        if (c == ',') {
            out += "&comma;";
        } else {
            out += c;
        }
    }
    return out;
}

const char* const kWhite = "Color: 0.000000,0.000000,1.000000";
}

std::vector<HeadEffect> Generate(const std::vector<long>& bars,
                                 const std::vector<Section>& sectionsIn,
                                 int headCount,
                                 const Params& p,
                                 const std::vector<int>& activeHeads) {
    std::vector<HeadEffect> result;
    if (bars.size() < 2 || headCount <= 0) {
        return result;
    }
    const int n = (int)bars.size() - 1;

    std::vector<Section> sections = sectionsIn;
    std::sort(sections.begin(), sections.end(), [](const Section& a, const Section& b) { return a.startMS < b.startMS; });

    std::vector<BarKind> kind(n, BarKind::Pool);
    std::vector<bool> peak(n, false);
    std::vector<int> barInSection(n, 0);
    for (int i = 0; i < n; ++i) {
        for (const auto& s : sections) {
            if (bars[i] >= s.startMS && bars[i] < s.endMS) {
                if (s.chorus) {
                    kind[i] = BarKind::Chorus;
                }
                peak[i] = s.peak;
                int firstBar = i;
                while (firstBar > 0 && bars[firstBar - 1] >= s.startMS) {
                    --firstBar;
                }
                barInSection[i] = i - firstBar;
                break;
            }
        }
    }
    for (size_t si = 1; si < sections.size(); ++si) {
        int best = 0;
        long bestDist = -1;
        for (int j = 0; j < n; ++j) {
            long d = std::labs(bars[j] - sections[si].startMS);
            if (bestDist < 0 || d < bestDist) {
                bestDist = d;
                best = j;
            }
        }
        kind[best] = BarKind::Sweep;
    }
    const bool hasOutro = n >= p.minBarsForOutro && n >= p.outroBars;
    if (hasOutro) {
        for (int i = n - p.outroBars; i < n; ++i) {
            kind[i] = BarKind::Outro;
        }
    }

    std::vector<std::vector<HeadEffect>> perHead(headCount);
    auto add = [&](int h, long s, long e, float t0, float t1, std::vector<float> dim) {
        if (e <= s) {
            return;
        }
        HeadEffect he;
        he.head = h;
        he.startMS = s;
        he.endMS = e;
        he.tilt0 = t0;
        he.tilt1 = t1;
        he.dimmer = std::move(dim);
        perHead[h].push_back(std::move(he));
    };

    int i = 0;
    while (i < n) {
        switch (kind[i]) {
        case BarKind::Pool: {
            int runEnd = i;
            while (runEnd < n && kind[runEnd] == BarKind::Pool) {
                ++runEnd;
            }
            const int chunk = std::max(1, p.poolBars);
            for (int c = i; c < runEnd; c += chunk) {
                int last = std::min(c + chunk, runEnd) - 1;
                for (int h = 0; h < headCount; ++h) {
                    float t = Fan(h, headCount) * p.poolTilt;
                    add(h, bars[c], bars[last + 1], t, t, { 0, 0, 0.5f, 0.6f, 1, 0 });
                }
            }
            i = runEnd;
            break;
        }
        case BarKind::Sweep:
            for (int h = 0; h < headCount; ++h) {
                float f = Fan(h, headCount);
                add(h, bars[i], bars[i + 1], f * p.sweepStartTilt, f * p.sweepEndTilt, { 0, 0, 0.15f, 1, 0.7f, 1, 1, 0 });
            }
            ++i;
            break;
        case BarKind::Chorus: {
            long len = bars[i + 1] - bars[i];
            long q[5];
            for (int k = 0; k <= 4; ++k) {
                q[k] = bars[i] + len * k / 4;
            }
            const int step = (p.chorusBeats == 1 || p.chorusBeats == 4) ? p.chorusBeats : 2;
            for (int k = 0; k < 4; k += step) {
                const int seg = k / step;
                const int globalSeg = i * (4 / step) + seg;
                long segStart = q[k];
                if (peak[i] && k == 0 && (barInSection[i] + std::max(1, p.burstEveryBars) - 1) % std::max(1, p.burstEveryBars) == 0) {
                    long end = std::min(q[0] + p.burstMS, q[1]);
                    for (int h = 0; h < headCount; ++h) {
                        add(h, q[0], end, 0, 0, { 1, 0, 0.12f, 0, 0.25f, 1, 0.37f, 0, 0.5f, 1, 0.62f, 0, 0.75f, 1, 1, 0 });
                    }
                    // The bank whose turn this is still plays the rest of the segment after the burst.
                    segStart = end;
                    if (q[k + step] - segStart < 150) {
                        continue;
                    }
                }
                float swing = ((i + seg) % 2 == 0) ? p.chorusSwing : -p.chorusSwing;
                for (int h = 0; h < headCount; ++h) {
                    int bank = p.chorusPairing == ChorusPairing::LeftRight ? (h < (headCount + 1) / 2 ? 0 : 1) : h % 2;
                    if (bank != globalSeg % 2) {
                        continue;
                    }
                    float base = Fan(h, headCount) * p.chorusTilt;
                    add(h, segStart, q[k + step], base - swing, base + swing, { 0, 1, 1, 0 });
                }
            }
            ++i;
            break;
        }
        case BarKind::Outro:
            for (int h = 0; h < headCount; ++h) {
                add(h, bars[i], bars[n], Fan(h, headCount) * p.outroTilt, 0, { 0, 0, 0.4f, 0.8f, 1, 0 });
            }
            i = n;
            break;
        }
    }

    for (int h = 0; h < headCount; ++h) {
        if (!activeHeads.empty() && std::find(activeHeads.begin(), activeHeads.end(), h) == activeHeads.end()) {
            continue;
        }
        auto& list = perHead[h];
        std::stable_sort(list.begin(), list.end(), [](const HeadEffect& a, const HeadEffect& b) { return a.startMS < b.startMS; });
        long cursor = 0;
        bool first = true;
        size_t prevIdx = 0;
        for (const auto& e : list) {
            long target = p.leadInMS;
            float poseDelta = 0.0f;
            float slew = h < (int)p.tiltSlewDegPerSec.size() ? p.tiltSlewDegPerSec[h] : 0.0f;
            if (!first) {
                poseDelta = std::fabs(e.tilt0 - result[prevIdx].tilt1);
                if (slew > 0.0f) {
                    target = (long)(poseDelta / slew * 1000.0f * 1.15f) + 50;
                }
            }
            // Touching effects with the same pose need no move; if the pose changes the head needs dark travel time.
            if (!first && poseDelta >= 1.0f && e.startMS - cursor < target) {
                // Not enough time for the head to reach its position: borrow time from the end of the previous beam.
                HeadEffect& prev = result[prevIdx];
                long dur = prev.endMS - prev.startMS;
                long keep = std::max<long>(100, (long)(dur * p.minBeamFraction));
                long newEnd = std::max(e.startMS - target, prev.startMS + keep);
                if (newEnd < prev.endMS) {
                    prev.endMS = newEnd;
                    cursor = newEnd;
                }
            }
            if (e.startMS - cursor >= p.minGapMS) {
                HeadEffect link;
                link.head = h;
                link.startMS = cursor;
                link.endMS = e.startMS;
                link.tilt0 = e.tilt0;
                link.tilt1 = e.tilt0;
                link.linked = true;
                result.push_back(std::move(link));
            }
            result.push_back(e);
            prevIdx = result.size() - 1;
            cursor = e.endMS;
            first = false;
        }
    }
    return result;
}

std::string BuildPalette() {
    return "C_BUTTON_Palette1=#FFFFFF,C_CHECKBOX_Palette1=1";
}

std::string BuildSettings(const HeadEffect& e, int fixture) {
    const bool linked = e.linked;
    const bool ramp = std::fabs(e.tilt1 - e.tilt0) >= 1.0f;

    std::string mh;
    if (linked) {
        // Mirrors MovingHeadPanel::ApplyLinkedHeadPosition: static position, dark for the whole effect.
        mh = "Pan: 90.0;Tilt: " + Fmt("%.1f", e.tilt0) + ";Dimmer: 0.0,0.0,1.0,0.0;" + kWhite + ";Heads: " + std::to_string(fixture);
    } else {
        mh = std::string(kWhite) + ";Dimmer: " + DimmerText(e.dimmer) + ";Pan: 90.0;";
        if (ramp) {
            mh += "Tilt VC: Active=TRUE|Id=ID_VALUECURVE_MHTilt|Type=Ramp|Min=-1800.00|Max=1800.00|P1=" +
                  Fmt("%.2f", e.tilt0 * 10.0f) + "|P2=" + Fmt("%.2f", e.tilt1 * 10.0f) + "|RV=TRUE|";
        } else {
            mh += "Tilt: " + Fmt("%.1f", e.tilt0);
        }
        mh += ";PanOffset: 0.0;TiltOffset: 0.0;Groupings: 1.0;Cycles: 1.0;Heads: " + std::to_string(fixture) + ";Shutter: On";
    }

    std::map<std::string, std::string> m = {
        { "B_CHOICE_BufferStyle", "Per Model Default" },
        { "E_CHECKBOX_AUTO_SHUTTER", linked ? "0" : "1" },
        { "E_CHECKBOX_MHIgnorePan", "0" },
        { "E_CHECKBOX_MHIgnoreTilt", "0" },
        { "E_CHECKBOX_MHLinkToNext", linked ? "1" : "0" },
        { "E_CHECKBOX_MHPatternEnable", "0" },
        { "E_CHECKBOX_MHShutterEnable", linked ? "0" : "1" },
        { "E_CHOICE_MHPattern", "Circle" },
        { "E_NOTEBOOK1", "Position" },
        { "E_NOTEBOOK2", "Color" },
        { "E_SLIDER_MHCycles", "10" },
        { "E_SLIDER_MHGroupings", "1" },
        { "E_SLIDER_MHPan", "900" },
        { "E_SLIDER_MHPanOffset", "0" },
        { "E_SLIDER_MHPathScale", "0" },
        { "E_SLIDER_MHPatternHeight", "45" },
        { "E_SLIDER_MHPatternPhaseOffset", "0" },
        { "E_SLIDER_MHPatternRotation", "0" },
        { "E_SLIDER_MHPatternStartOffset", "0" },
        { "E_SLIDER_MHPatternWidth", "90" },
        { "E_SLIDER_MHPatternXOffset", "0" },
        { "E_SLIDER_MHPatternYOffset", "0" },
        { "E_SLIDER_MHTilt", ramp && !linked ? "0" : std::to_string((int)std::lround(e.tilt0 * 10.0f)) },
        { "E_SLIDER_MHTiltOffset", "0" },
        { "E_SLIDER_MHTimeOffset", "0" },
        { "E_TEXTCTRL_MHPathDef", "" },
        { "T_CHECKBOX_LayerMorph", "0" },
        { "T_CHOICE_LayerMethod", "Normal" },
        { "T_SLIDER_EffectLayerMix", "0" },
    };
    for (int i = 1; i <= 8; ++i) {
        m["E_TEXTCTRL_MH" + std::to_string(i) + "_Settings"] = (i == fixture) ? EscapeCommas(mh) : "";
    }

    std::string out;
    for (const auto& kv : m) {
        if (!out.empty()) {
            out += ",";
        }
        out += kv.first + "=" + kv.second;
    }
    return out;
}

}
