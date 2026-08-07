/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SnowflakesEffect.h"

#include <cassert>
#include <cstdint>
#include <vector>

#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"

#include "../../include/snowflakes-16.xpm"
#include "../../include/snowflakes-24.xpm"
#include "../../include/snowflakes-32.xpm"
#include "../../include/snowflakes-48.xpm"
#include "../../include/snowflakes-64.xpm"

// Fallback defaults (replaced from Snowflakes.json in OnMetadataLoaded).
int SnowflakesEffect::sCountDefault = 5;
int SnowflakesEffect::sCountMin = 1;
int SnowflakesEffect::sCountMax = 100;
int SnowflakesEffect::sTypeDefault = 1;
int SnowflakesEffect::sSpeedDefault = 10;
int SnowflakesEffect::sSpeedMin = 0;
int SnowflakesEffect::sSpeedMax = 50;
std::string SnowflakesEffect::sFallingDefault = "Driving";
int SnowflakesEffect::sWarmupFramesDefault = 0;

SnowflakesEffect::SnowflakesEffect(int id) : RenderableEffect(id, "Snowflakes", snowflakes_16, snowflakes_24, snowflakes_32, snowflakes_48, snowflakes_64)
{
    tooltip = "Snow Flakes";
}

SnowflakesEffect::~SnowflakesEffect()
{
    //dtor
}

void SnowflakesEffect::OnMetadataLoaded()
{
    sCountDefault = GetIntDefault("Snowflakes_Count", sCountDefault);
    sCountMin = (int)GetMinFromMetadata("Snowflakes_Count", sCountMin);
    sCountMax = (int)GetMaxFromMetadata("Snowflakes_Count", sCountMax);
    sTypeDefault = GetIntDefault("Snowflakes_Type", sTypeDefault);
    sSpeedDefault = GetIntDefault("Snowflakes_Speed", sSpeedDefault);
    sSpeedMin = (int)GetMinFromMetadata("Snowflakes_Speed", sSpeedMin);
    sSpeedMax = (int)GetMaxFromMetadata("Snowflakes_Speed", sSpeedMax);
    sFallingDefault = GetStringDefault("Falling", sFallingDefault);
    sWarmupFramesDefault = GetIntDefault("Snowflakes_WarmupFrames", sWarmupFramesDefault);
}

int static possible_downward_moves(RenderBuffer &buffer, int x, int y)
{
    int moves = 0;

    // no moves possible from bottom row
    if (y == 0) {
        return 0;
    }

    if (buffer.GetTempPixel(x-1 < 0 ? x-1+buffer.BufferWi : x-1, y-1) == xlBLACK) {
        moves += 1;
    }
    if (buffer.GetTempPixel(x, y-1) == xlBLACK) {
        moves += 2;
    }
    if (buffer.GetTempPixel(x+1 >= buffer.BufferWi ? x+1-buffer.BufferWi : x+1, y-1) == xlBLACK) {
        moves += 4;
    }

    return moves;
}

static void set_pixel_if_not_color(RenderBuffer &buffer, int x, int y, xlColor toColor, xlColor notColor, bool wrapx, bool wrapy)
{
    int adjx = x;
    int adjy = y;

    if (x < 0) {
        if (wrapx) {
            adjx += buffer.BufferWi;
        }
        else {
            return;
        }
    }
    else if (x >= buffer.BufferWi) {
        if (wrapx) {
            adjx -= buffer.BufferWi;
        }
        else {
            return;
        }
    }
    if (y < 0) {
        if (wrapy) {
            adjy += buffer.BufferHt;
        }
        else {
            return;
        }
    }
    else if (y >= buffer.BufferHt) {
        if (wrapy) {
            adjy -= buffer.BufferHt;
        }
        else {
            return;
        }
    }

    // strip off alpha when comparing
    if (buffer.GetTempPixel(adjx, adjy).GetRGB() != notColor.GetRGB()) {
        buffer.SetPixel(adjx, adjy, toColor);
    }
}

class SnowflakesRenderCache : public EffectRenderCache {
public:
    SnowflakesRenderCache() : LastSnowflakeCount(0), LastSnowflakeType(0), LastFalling(""), effectState(0) {};
    virtual ~SnowflakesRenderCache() {};

    int LastSnowflakeCount;
    int LastSnowflakeType;
    std::string LastFalling;
    int effectState;
};

// Tier-2 immutable per-frame draw snapshot, produced by AdvanceState AFTER the
// whole per-frame state transition has run (init when triggered + warmup/move,
// all of which consume RNG).  Holds everything Render needs to rasterise the
// frame as a pure function of the snapshot: the post-advance temp pixel layer
// (the falling-flake positions), the two palette draw colors, the driving scroll
// offset, and - critically - the pre-rolled type-2 orientation decisions.
//
// The legacy draw consumed RNG mid-rasterise (one randInt(0,99) per non-settled
// "3 nodes" (type-2) flake, in pixel-scan order).  To keep Render RNG-free - so
// the serial advance-then-draw stream stays byte-identical to the frame-parallel
// clone stream (which reads the RNG from offset 0) - AdvanceState PRE-ROLLS those
// exact decisions, walking the draw's scan order and roll conditions precisely,
// and bakes them here; Render consumes them sequentially instead of calling
// randInt.
struct SnowflakesFrameState : public EffectFrameState {
    xlColorVector tempbuf;        // post-advance flake layer
    xlColor color1;               // palette color 0 (draw)
    xlColor color2;               // palette color 1 (draw)
    int movement = 0;             // driving-mode scroll offset for this frame
    bool driving = false;         // CHOICE_Falling == "Driving"
    std::vector<uint8_t> rolls;   // baked type-2 orientation rolls (>50 => 1), draw-scan order
};

RenderableEffect::FrameParallelism SnowflakesEffect::GetFrameParallelism(const SettingsMap& settings) const {
    // All cross-frame state lives in the cache scalars (LastSnowflakeCount/Type,
    // LastFalling, effectState) plus the temp pixel layer holding the falling-
    // snowflake positions.  AdvanceState advances both serially and returns a
    // self-contained draw snapshot; no mode reads any other cross-frame resource
    // (audio, timing track, loaded image), so a parallel draw pass reproduces
    // each frame from that snapshot.
    return FrameParallelism::Snapshottable;
}

void SnowflakesEffect::MoveFlakes(RenderBuffer& buffer, int snowflakeType, const std::string& falling, int count, const xlColor& color1, int& effectState)
{
    int starty = 0;
    if (falling == "Falling & Accumulating") {
        starty = 1;
    }

    for (int x = 0; x < buffer.BufferWi; x++) {
        for (int y = starty; y < buffer.BufferHt; y++) {
                // if there is a flake to move
                xlColor color3;
                buffer.GetTempPixel(x, y, color3);
                if (color3 != xlBLACK) {
                    // check where we can move to?
                    int moves = possible_downward_moves(buffer, x, y);

                    // we have something to move
                    //  randomly move the flake left or right
                    if (moves > 0 || (falling == "Falling" && y == 0)) {
                        int x0;
                        switch (buffer.randInt(0, 8)) {
                        case 0:
                            if (moves & 1) {
                                x0 = x - 1;
                            } else {
                                if (moves & 2) {
                                    x0 = x;
                                } else {
                                    x0 = x + 1;
                                }
                            }
                            break;
                        case 1:
                            if (moves & 4) {
                                x0 = x + 1;
                            } else {
                                if (moves & 2) {
                                    x0 = x;
                                } else {
                                    x0 = x - 1;
                                }
                            }
                            break;
                        default: // down more often then left/right to look less "jittery"
                            if (moves & 2) {
                                x0 = x;
                            } else if ((moves & 5) == 4) {
                                x0 = x + 1;
                            } else if ((moves & 5) == 1) {
                                x0 = x - 1;
                            } else {
                                switch (buffer.randInt(0, 1)) {
                                case 0:
                                    x0 = x + 1;
                                    break;
                                default:
                                    x0 = x - 1;
                                    break;
                                }
                            }
                            break;
                        }

                        // handle wrap around
                        if (x0 < 0) {
                            x0 += buffer.BufferWi;
                        } else if (x0 >= buffer.BufferWi) {
                            x0 -= buffer.BufferWi;
                        }

                        // and move it down
                        int y0 = y - 1;

                        buffer.SetTempPixel(x, y, xlBLACK);
                        if (y0 >= 0) {
                            // move the flake down
                            buffer.SetTempPixel(x0, y0, color3);

                            if (falling == "Falling & Accumulating") {
                                int nextmoves = possible_downward_moves(buffer, x0, y0);
                                if (nextmoves == 0) {
                                    // we cant move any further so we can add one at the top
                                    effectState--;
                                }
                            }
                        } else {
                            // falling should always be just falling ... never accumulate
                            effectState--;
                        }
                    }
                }
        }
    }

    // add new flakes to the top
    int check = 0;
    int placedFullCount = 0;
    while (effectState < count && check < 20) {
        // find unused space
        int x = buffer.randInt(0, buffer.BufferWi - 1);
        if (buffer.GetTempPixel(x, buffer.BufferHt - 1) == xlBLACK) {
            effectState++;
            buffer.SetTempPixel(x, buffer.BufferHt - 1, color1, snowflakeType == 0 ? buffer.randInt(0, 8) : snowflakeType - 1);

            int nextmoves = possible_downward_moves(buffer, x, buffer.BufferHt - 1);
            if (nextmoves == 0) {
                // the placed pixel fills the column, make sure we note that so we can place
                // another snowflake
                placedFullCount++;
            }
        }
        check++;
    }
    effectState -= placedFullCount;
}

// A type-2 ("3 nodes") flake has "settled" when the whole column below it (rows
// 0 .. y-2) is filled.  Used identically by AdvanceState's pre-roll and the draw
// so their per-flake roll decisions stay in lockstep.
static bool flakeIsAtBottom(RenderBuffer& buffer, int x, int y) {
    for (int yt = 0; yt < y - 1; yt++) {
        if (buffer.GetTempPixel(x, yt) == xlBLACK) {
            return false;
        }
    }
    return true;
}

// Pure, RNG-free rasterise of a frame's draw snapshot.  Loads the post-advance
// flake layer, then paints exactly as the legacy draw did - except the type-2
// orientation choice, which reads the pre-rolled decisions from fs.rolls in the
// same pixel-scan order AdvanceState baked them.
static void DrawSnowflakes(RenderBuffer& buffer, const SnowflakesFrameState& fs) {
    // Load the flake layer this snapshot carries.  Essential in the frame-parallel
    // draw pass (the clone buffer's temp layer is otherwise stale); a harmless
    // idempotent copy in the serial path (AdvanceState already advanced it here).
    buffer.SetTempBufVector(fs.tempbuf);

    const bool wrapx = false; // snowflakes do not draw wrapped near the x edges
    const xlColor c1(0, 1, 0);
    const xlColor c2(0, 0, 1);
    const xlColor& color1 = fs.color1;
    const xlColor& color2 = fs.color2;

    if (fs.driving) {
        const int movement = fs.movement;
        for (int x = 0; x < buffer.BufferWi; x++) {
            int new_x = (x + movement / 20) % buffer.BufferWi;  // CW
            int new_x2 = (x - movement / 20) % buffer.BufferWi; // CCW
            if (new_x2 < 0)
                new_x2 += buffer.BufferWi;
            for (int y = 0; y < buffer.BufferHt; y++) {
                int new_y = (y + movement / 10) % buffer.BufferHt;
                int new_y2 = (new_y + buffer.BufferHt / 2) % buffer.BufferHt;
                xlColor color3;
                buffer.GetTempPixel(new_x, new_y, color3);
                if (color3 == xlBLACK)
                    buffer.GetTempPixel(new_x2, new_y2, color3); // strip off the alpha channel
                if (color3 == c1) {
                    buffer.SetPixel(x, y, color1);
                } else if (color3 == c2) {
                    buffer.SetPixel(x, y, color2);
                }
            }
        }
        return;
    }

    // paint my current state
    size_t rollIdx = 0;
    for (int y = 0; y < buffer.BufferHt; y++) {
        for (int x = 0; x < buffer.BufferWi; x++) {
            xlColor color3;
            buffer.GetTempPixel(x, y, color3);
            if (color3 != xlBLACK) {
                // draw flake, SnowflakeType=0 is random type
                switch (color3.Alpha()) {
                case 0:
                    // single node
                    buffer.SetPixel(x, y, color1);
                    break;
                case 1:
                    // 5 nodes
                    buffer.SetPixel(x, y, color1);
                    set_pixel_if_not_color(buffer, x - 1, y, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x + 1, y, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x, y - 1, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x, y + 1, color2, color1, wrapx, false);
                    break;
                case 2: {
                    // 3 nodes
                    buffer.SetPixel(x, y, color1);
                    // when flake has settled always paint it horizontally, else
                    // paint per the pre-rolled orientation decision
                    if (flakeIsAtBottom(buffer, x, y)) {
                        set_pixel_if_not_color(buffer, x - 1, y, color2, color1, wrapx, false);
                        set_pixel_if_not_color(buffer, x + 1, y, color2, color1, wrapx, false);
                    } else {
                        if (fs.rolls[rollIdx++]) {
                            set_pixel_if_not_color(buffer, x - 1, y, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x + 1, y, color2, color1, wrapx, false);
                        } else {
                            set_pixel_if_not_color(buffer, x, y - 1, color2, color1, wrapx, false);
                            set_pixel_if_not_color(buffer, x, y + 1, color2, color1, wrapx, false);
                        }
                    }
                } break;
                case 3:
                    // 9 nodes
                    buffer.SetPixel(x, y, color1);
                    for (uint8_t i = 1; i <= 2; i++) {
                        set_pixel_if_not_color(buffer, x - i, y, color2, color1, wrapx, false);
                        set_pixel_if_not_color(buffer, x + i, y, color2, color1, wrapx, false);
                        set_pixel_if_not_color(buffer, x, y - i, color2, color1, wrapx, false);
                        set_pixel_if_not_color(buffer, x, y + i, color2, color1, wrapx, false);
                    }
                    break;
                case 4:
                    // 13 nodes
                    buffer.SetPixel(x, y, color1);
                    set_pixel_if_not_color(buffer, x - 1, y, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x + 1, y, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x, y + 1, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x, y - 1, color2, color1, wrapx, false);

                    set_pixel_if_not_color(buffer, x - 1, y + 2, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x + 1, y + 2, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x - 1, y - 2, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x + 1, y - 2, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x + 2, y - 1, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x + 2, y + 1, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x - 2, y - 1, color2, color1, wrapx, false);
                    set_pixel_if_not_color(buffer, x - 2, y + 1, color2, color1, wrapx, false);
                    break;
                case 5:
                    buffer.SetPixel(x, y, color1);
                    buffer.SetPixel(x + 1, y, color1);
                    buffer.SetPixel(x + 1, y + 1, color1);
                    buffer.SetPixel(x, y + 1, color1);
                    break;
                case 6:
                    buffer.SetPixel(x, y, color1);
                    buffer.SetPixel(x + 1, y, color1);
                    buffer.SetPixel(x, y + 1, color1);
                    buffer.SetPixel(x - 1, y, color1);
                    buffer.SetPixel(x, y - 1, color1);
                    break;
                case 7:
                    buffer.SetPixel(x, y + 2, color1);

                    buffer.SetPixel(x - 1, y + 1, color1);
                    buffer.SetPixel(x, y + 1, color1);
                    buffer.SetPixel(x + 1, y + 1, color1);

                    buffer.SetPixel(x - 2, y, color1);
                    buffer.SetPixel(x - 1, y, color1);
                    buffer.SetPixel(x, y, color1);
                    buffer.SetPixel(x + 1, y, color1);
                    buffer.SetPixel(x + 2, y, color1);

                    buffer.SetPixel(x - 1, y - 1, color1);
                    buffer.SetPixel(x, y - 1, color1);
                    buffer.SetPixel(x + 1, y - 1, color1);

                    buffer.SetPixel(x, y - 2, color1);
                    break;
                case 8:
                    buffer.SetPixel(x, y, color1);
                    buffer.SetPixel(x + 1, y + 1, color1);
                    buffer.SetPixel(x - 1, y + 1, color1);
                    buffer.SetPixel(x - 1, y - 1, color1);
                    buffer.SetPixel(x + 1, y - 1, color1);
                    break;
                case 9:
                    break;
                default:
                    break;
                }
            }
        }
    }
    // Every baked roll must be consumed exactly once - a mismatch means the pre-
    // roll scan diverged from the draw scan (wrong output vs the release).
    assert(rollIdx == fs.rolls.size());
}

void SnowflakesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    // Tier-2 draw pass: rasterise the snapshot AdvanceState produced.  Under the
    // tier-2 engine this is the ONLY path reached (AdvanceState runs first and
    // sets pendingSnapshot in both serial and frame-parallel rendering).
    if (buffer.pendingSnapshot != nullptr) {
        DrawSnowflakes(buffer, static_cast<const SnowflakesFrameState&>(*buffer.pendingSnapshot));
        return;
    }
    // Defensive fall-through for any caller that invokes Render without first
    // going through AdvanceState: advance then draw.  The draw is a pure function
    // of the snapshot, so this stays byte-identical.
    auto fs = AdvanceState(effect, SettingsMap, buffer);
    DrawSnowflakes(buffer, static_cast<const SnowflakesFrameState&>(*fs));
}

std::unique_ptr<EffectFrameState> SnowflakesEffect::AdvanceState(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Count = GetValueCurveInt("Snowflakes_Count", sCountDefault, SettingsMap, oset, sCountMin, sCountMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int SnowflakeType = SettingsMap.GetInt("SLIDER_Snowflakes_Type", sTypeDefault);
    int sSpeed = GetValueCurveInt("Snowflakes_Speed", sSpeedDefault, SettingsMap, oset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int warmupFrames = SettingsMap.GetInt("SLIDER_Snowflakes_WarmupFrames", sWarmupFramesDefault);

    std::string falling = SettingsMap.Get("CHOICE_Falling", sFallingDefault);

    const xlColor c1(0, 1, 0);
    const xlColor c2(0, 0, 1);

    SnowflakesRenderCache *cache = (SnowflakesRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new SnowflakesRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &LastSnowflakeCount = cache->LastSnowflakeCount;
    int &LastSnowflakeType = cache->LastSnowflakeType;
    int &effectState = cache->effectState;
    std::string& LastFalling = cache->LastFalling;

    xlColor color1, color2;
    buffer.palette.GetColor(0, color1);
    buffer.palette.GetColor(1, color2);

    if (buffer.needToInit ||
        (Count != LastSnowflakeCount && falling == "Driving") ||
        SnowflakeType != LastSnowflakeType ||
        falling != LastFalling) {

        // initialize
        buffer.needToInit = false;
        LastSnowflakeCount = Count;
        LastSnowflakeType = SnowflakeType;
        LastFalling = falling;
        buffer.ClearTempBuf();
        effectState = 0;

        // place Count snowflakes
        for (int n = 0; n < Count; n++) {

            int delta_y = buffer.BufferHt / 4;
            int y0 = (n % 4)*delta_y;

            if (y0 + delta_y > buffer.BufferHt) delta_y = buffer.BufferHt - y0;
            if (delta_y < 1) delta_y = 1;

            int x = 0;
            int y = 0;

            // find unused space
            for (int check = 0; check < 20; check++)
            {
                x = buffer.randInt(0, buffer.BufferWi - 1);
                y = y0 + buffer.randInt(0, delta_y - 1);
                if (buffer.GetTempPixel(x, y) == xlBLACK) {
                    effectState++;
                    break;
                }
            }

            // draw flake, SnowflakeType=0 is random type
            switch (SnowflakeType == 0 ? buffer.randInt(0, 8) : SnowflakeType - 1)
            {
            case 0:
                // single node
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 0);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                }
                break;
            case 1:
                // 5 nodes
                if (x < 1) x += 1;
                if (y < 1) y += 1;
                if (x > buffer.BufferWi - 2) x -= 1;
                if (y > buffer.BufferHt - 2) y -= 1;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 1);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                    buffer.SetTempPixel(x - 1, y, c2);
                    buffer.SetTempPixel(x + 1, y, c2);
                    buffer.SetTempPixel(x, y - 1, c2);
                    buffer.SetTempPixel(x, y + 1, c2);
                }
                break;
            case 2:
                // 3 nodes
                if (x < 1) x += 1;
                if (y < 1) y += 1;
                if (x > buffer.BufferWi - 2) x -= 1;
                if (y > buffer.BufferHt - 2) y -= 1;
                if (falling != "Driving")
                {
                    buffer.SetTempPixel(x, y, color1, 2);
                }
                else
                {
                    buffer.SetTempPixel(x, y, c1);
                    if (buffer.randInt(0, 99) > 50)      // % 2 was not so random
                    {
                        buffer.SetTempPixel(x - 1, y, c2);
                        buffer.SetTempPixel(x + 1, y, c2);
                    }
                    else
                    {
                        buffer.SetTempPixel(x, y - 1, c2);
                        buffer.SetTempPixel(x, y + 1, c2);
                    }
                }
                break;
            case 3:
                // 9 nodes
                if (x < 2) x += 2;
                if (y < 2) y += 2;
                if (x > buffer.BufferWi - 3) x -= 2;
                if (y > buffer.BufferHt - 3) y -= 2;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 3);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                    for (uint8_t i = 1; i <= 2; i++)
                    {
                        buffer.SetTempPixel(x - i, y, c2);
                        buffer.SetTempPixel(x + i, y, c2);
                        buffer.SetTempPixel(x, y - i, c2);
                        buffer.SetTempPixel(x, y + i, c2);
                    }
                }
                break;
            case 4:
                // 13 nodes
                if (x < 2) x += 2;
                if (y < 2) y += 2;
                if (x > buffer.BufferWi - 3) x -= 2;
                if (y > buffer.BufferHt - 3) y -= 2;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 4);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                    buffer.SetTempPixel(x - 1, y, c2);
                    buffer.SetTempPixel(x + 1, y, c2);
                    buffer.SetTempPixel(x, y - 1, c2);
                    buffer.SetTempPixel(x, y + 1, c2);

                    buffer.SetTempPixel(x - 1, y + 2, c2);
                    buffer.SetTempPixel(x + 1, y + 2, c2);
                    buffer.SetTempPixel(x - 1, y - 2, c2);
                    buffer.SetTempPixel(x + 1, y - 2, c2);
                    buffer.SetTempPixel(x + 2, y - 1, c2);
                    buffer.SetTempPixel(x + 2, y + 1, c2);
                    buffer.SetTempPixel(x - 2, y - 1, c2);
                    buffer.SetTempPixel(x - 2, y + 1, c2);
                }
                break;
            case 5:
                if (x > buffer.BufferWi - 2) x -= 1;
                if (y > buffer.BufferHt - 2) y -= 1;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 5);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                    buffer.SetTempPixel(x + 1, y, c1);
                    buffer.SetTempPixel(x + 1, y + 1, c1);
                    buffer.SetTempPixel(x, y + 1, c1);
                }
                break;
            case 6:
                if (x < 1) x += 1;
                if (y < 1) y += 1;
                if (x > buffer.BufferWi - 2) x -= 1;
                if (y > buffer.BufferHt - 2) y -= 1;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 6);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                    buffer.SetTempPixel(x + 1, y, c1);
                    buffer.SetTempPixel(x, y + 1, c1);
                    buffer.SetTempPixel(x - 1, y, c1);
                    buffer.SetTempPixel(x, y - 1, c1);
                }
                break;
            case 7:
                if (x < 2) x += 2;
                if (y < 2) y += 2;
                if (x > buffer.BufferWi - 3) x -= 2;
                if (y > buffer.BufferHt - 3) y -= 2;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 7);
                }
                else {
                    buffer.SetTempPixel(x, y+2, c1);
                    
                    buffer.SetTempPixel(x-1, y + 1, c1);
                    buffer.SetTempPixel(x, y + 1, c1);
                    buffer.SetTempPixel(x+1, y + 1, c1);

                    buffer.SetTempPixel(x - 2, y, c1);
                    buffer.SetTempPixel(x - 1, y, c1);
                    buffer.SetTempPixel(x, y, c1);
                    buffer.SetTempPixel(x + 1, y, c1);
                    buffer.SetTempPixel(x +2, y, c1);

                    buffer.SetTempPixel(x - 1, y - 1, c1);
                    buffer.SetTempPixel(x, y - 1, c1);
                    buffer.SetTempPixel(x + 1, y - 1, c1);

                    buffer.SetTempPixel(x, y - 2, c1);
                }
                break;
            case 8:
                if (x < 1) x += 1;
                if (y < 1) y += 1;
                if (x > buffer.BufferWi - 2) x -= 1;
                if (y > buffer.BufferHt - 2) y -= 1;
                if (falling != "Driving") {
                    buffer.SetTempPixel(x, y, color1, 8);
                }
                else {
                    buffer.SetTempPixel(x, y, c1);
                    buffer.SetTempPixel(x + 1, y+1, c1);
                    buffer.SetTempPixel(x-1, y + 1, c1);
                    buffer.SetTempPixel(x - 1, y-1, c1);
                    buffer.SetTempPixel(x+1, y - 1, c1);
                }
                break;
            case 9:
                // 45 nodes (not enabled)
                break;
            default:
                break;
            }
        }
    }

    // move snowflakes
    int movement = (buffer.curPeriod - buffer.curEffStartPer) * sSpeed * buffer.frameTimeInMs / 50;
    bool driving = falling == "Driving";

    if (!driving && buffer.curPeriod == buffer.curEffStartPer)
    {
        for (int i = 0; i < warmupFrames; ++i)
        {
            // this controls the speed by skipping movement when slow
            if ((i * (sSpeed + 1)) / 30 != ((i - 1) * (sSpeed + 1)) / 30) {
                MoveFlakes(buffer, SnowflakeType, falling, Count, color1, effectState);
            }
        }
    } else if (!driving)
    {
        // this controls the speed by skipping movement when slow
        if (((buffer.curPeriod - buffer.curEffStartPer) * (sSpeed + 1)) / 30 != ((buffer.curPeriod - buffer.curEffStartPer - 1) * (sSpeed + 1)) / 30) {
            MoveFlakes(buffer, SnowflakeType, falling, Count, color1, effectState);
        }
    }

    // Bake this frame's draw-time RNG decisions now, so Render is a pure, RNG-
    // free rasterise of the snapshot.  Only the non-driving draw rolls: one
    // randInt(0,99) per non-settled "3 nodes" (type-2) flake, walked in the exact
    // draw-scan order (y outer, x inner) and gated by the exact same conditions
    // DrawSnowflakes uses (non-black, alpha == 2, not settled).  Driving mode's
    // draw is a pure temp-buffer scan and never rolls.
    auto fs = std::make_unique<SnowflakesFrameState>();
    fs->driving = driving;
    fs->movement = movement;
    fs->color1 = color1;
    fs->color2 = color2;
    if (!driving) {
        for (int y = 0; y < buffer.BufferHt; y++) {
            for (int x = 0; x < buffer.BufferWi; x++) {
                xlColor color3;
                buffer.GetTempPixel(x, y, color3);
                if (color3 != xlBLACK && color3.Alpha() == 2 && !flakeIsAtBottom(buffer, x, y)) {
                    fs->rolls.push_back(buffer.randInt(0, 99) > 50 ? 1 : 0); // % 2 was not so random
                }
            }
        }
    }
    fs->tempbuf = buffer.GetTempBufVector();
    return fs;
}
