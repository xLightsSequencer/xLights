/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "SnowflakesEffect.h"
#include "SnowflakesPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/snowflakes-16.xpm"
#include "../../include/snowflakes-24.xpm"
#include "../../include/snowflakes-32.xpm"
#include "../../include/snowflakes-48.xpm"
#include "../../include/snowflakes-64.xpm"

SnowflakesEffect::SnowflakesEffect(int id) : RenderableEffect(id, "Snowflakes", snowflakes_16, snowflakes_24, snowflakes_32, snowflakes_48, snowflakes_64)
{
    tooltip = "Snow Flakes";
}

SnowflakesEffect::~SnowflakesEffect()
{
    //dtor
}

xlEffectPanel *SnowflakesEffect::CreatePanel(wxWindow *parent) {
    return new SnowflakesPanel(parent);
}

bool SnowflakesEffect::needToAdjustSettings(const std::string &version)
{
    return IsVersionOlder("4.3.03", version);
}

void SnowflakesEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    SettingsMap &settings = effect->GetSettings();
    bool accumulate = settings.GetBool("E_CHECKBOX_Snowflakes_Accumulate", false);

    // if it was accumulate then clear it and change the falling type from the default
    if (accumulate)
    {
        settings["E_CHOICE_Falling"] = "Falling & Accumulating";
        settings.erase("E_CHECKBOX_Snowflakes_Accumulate");
    }

    // if it was not accumulate then it should be driving
    bool accumulate2 = settings.GetBool("E_CHECKBOX_Snowflakes_Accumulate", true);
    if (!accumulate2)
    {
        settings["E_CHOICE_Falling"] = "Driving";
        settings.erase("E_CHECKBOX_Snowflakes_Accumulate");
    }

    // also give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }
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

void SnowflakesEffect::SetDefaultParameters()
{
    SnowflakesPanel *sp = (SnowflakesPanel*)panel;
    if (sp == nullptr) {
        return;
    }

    sp->BitmapButton_Snowflakes_Count->SetActive(false);
    sp->BitmapButton_Snowflakes_Speed->SetActive(false);

    SetSliderValue(sp->Slider_Snowflakes_Count, 5);
    SetSliderValue(sp->Slider_Snowflakes_Type, 1);
    SetSliderValue(sp->Slider_Snowflakes_Speed, 10);
    SetChoiceValue(sp->Choice_Falling, "Driving");
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
                        switch (rand() % 9) {
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
                                switch (rand() % 2) {
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
        int x = rand() % buffer.BufferWi;
        if (buffer.GetTempPixel(x, buffer.BufferHt - 1) == xlBLACK) {
            effectState++;
            buffer.SetTempPixel(x, buffer.BufferHt - 1, color1, snowflakeType == 0 ? rand() % 9 : snowflakeType - 1);

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

void SnowflakesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {

    float oset = buffer.GetEffectTimeIntervalPosition();
    int Count = GetValueCurveInt("Snowflakes_Count", 5, SettingsMap, oset, SNOWFLAKES_COUNT_MIN, SNOWFLAKES_COUNT_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int SnowflakeType = SettingsMap.GetInt("SLIDER_Snowflakes_Type", 1);
    int sSpeed = GetValueCurveInt("Snowflakes_Speed", 10, SettingsMap, oset, SNOWFLAKES_SPEED_MIN, SNOWFLAKES_SPEED_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int warmupFrames = SettingsMap.GetInt("SLIDER_Snowflakes_WarmupFrames", 0);

    bool wrapx = false; // set to true if you want snowflakes to draw wrapped around when near edges in the accumulate effect.
    std::string falling = SettingsMap.Get("CHOICE_Falling", "Driving");

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
                x = rand() % buffer.BufferWi;
                y = y0 + (rand() % delta_y);
                if (buffer.GetTempPixel(x, y) == xlBLACK) {
                    effectState++;
                    break;
                }
            }

            // draw flake, SnowflakeType=0 is random type
            switch (SnowflakeType == 0 ? rand() % 9 : SnowflakeType - 1)
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
                    if (rand() % 100 > 50)      // % 2 was not so random
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

    if (driving)
    {
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
    }
    else
    {
        // paint my current state
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
                        bool isAtBottom = true;
                        for (int yt = 0; yt < y - 1; yt++) {
                                if (buffer.GetTempPixel(x, yt) == xlBLACK) {
                                    isAtBottom = false;
                                    break;
                                }
                        }

                        // when flake has settled always paint it horizontally
                        if (isAtBottom) {
                                set_pixel_if_not_color(buffer, x - 1, y, color2, color1, wrapx, false);
                                set_pixel_if_not_color(buffer, x + 1, y, color2, color1, wrapx, false);
                        } else {
                                if (rand() % 100 > 50) // % 2 was not so random
                                {
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
    }
}
