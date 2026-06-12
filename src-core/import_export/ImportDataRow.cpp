/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ImportDataRow.h"

#include <cctype>

#include <spdlog/fmt/fmt.h>

#include "render/EffectLayer.h"

namespace {

bool isOnLine(int x1, int y1, int x2, int y2, int x3, int y3)
{
    double diffx = x2 - x1;
    double diffy = y2 - y1;
    double b = y1 - diffy / diffx * x1;
    double ye1 = diffy / diffx * x3 + b;

    return (y3 + 1) >= ye1 && (y3 - 1) <= ye1;
}

bool isOnLineColor(const xlColor& v1, const xlColor& v2, const xlColor& v3, int x, int x2, int x3)
{
    return isOnLine(x, v1.Red(), x2, v2.Red(), x3, v3.Red()) && isOnLine(x, v1.Green(), x2, v2.Green(), x3, v3.Green()) && isOnLine(x, v1.Blue(), x2, v2.Blue(), x3, v3.Blue());
}

int RampLenColor(int start, std::vector<xlColor>& colors)
{
    int s = start + 2;
    for (; s < (int)colors.size(); s++) {
        if (!isOnLineColor(colors[start], colors[s - 1], colors[s], start, s - 1, s)) {
            return s - start;
        }
    }
    if (s == (int)colors.size()) {
        return s - start;
    }
    return 0;
}

bool is_base64(unsigned char c)
{
    return (std::isalnum(c) || (c == '+') || (c == '/'));
}

} // namespace

void ConvertDataRowToEffects(EffectLayer* layer, xlColorVector& colors, int frameTime, bool eraseExisting)
{
    if (eraseExisting) {
        layer->DeleteAllEffects();
    }

    colors.push_back(xlBLACK);
    int startTime = 0;
    xlColor lastColor(xlBLACK);

    for (size_t x = 0; x < colors.size() - 3; ++x) {
        if (colors[x] != colors[x + 1]) {
            int len = RampLenColor(x, colors);
            if (len >= 3) {
                HSVValue v1 = colors[x].asHSV();
                HSVValue v2 = colors[x + len - 1].asHSV();

                int stime = x * frameTime;
                int etime = (x + len) * frameTime;
                if (colors[x] == xlBLACK || colors[x + len - 1] == xlBLACK || (v1.hue == v2.hue)) {
                    HSVValue c = colors[x].asHSV();
                    if (colors[x] == xlBLACK) {
                        c = colors[x + len - 1].asHSV();
                    }
                    c.value = 1.0;
                    xlColor c2(c);

                    int i = v1.value * 100.0;
                    int i2 = v2.value * 100.0;
                    std::string settings = fmt::format("E_TEXTCTRL_Eff_On_Start={},E_TEXTCTRL_Eff_On_End={}", i, i2);
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)c2 + ",C_CHECKBOX_Palette1=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime)) {
                        layer->AddEffect(0, "On", settings, palette, stime, etime, false, false);
                    }
                } else {
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)colors[x] + ",C_CHECKBOX_Palette1=1,"
                                                                                          "C_BUTTON_Palette2=" +
                                          (std::string)colors[x + len - 1] + ",C_CHECKBOX_Palette2=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime)) {
                        layer->AddEffect(0, "Color Wash", "", palette, stime, etime, false, false);
                    }
                }
                for (int z = 0; z < len; ++z) {
                    colors[x + z] = xlBLACK;
                }
            }
        }
    }

    for (size_t x = 0; x < colors.size(); ++x) {
        if (lastColor != colors[x]) {
            int time = x * frameTime;
            if (lastColor != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)lastColor + ",C_CHECKBOX_Palette1=1";

                if (time != startTime) {
                    if (!layer->HasEffectsInTimeRange(startTime, time)) {
                        layer->AddEffect(0, "On", "", palette, startTime, time, false, false);
                    }
                }
            }
            startTime = time;
            lastColor = colors[x];
        }
    }
}

int Base64Decode(const std::string& encoded, std::vector<unsigned char>& data)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t in_len = encoded.size();
    int i = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded[in_] != '=') && is_base64(encoded[in_])) {
        char_array_4[i++] = encoded[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) {
                data.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if (i && in_ < (int)encoded.size() && encoded[in_] == '=') {
        for (int j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (int j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int j = 0; (j < i - 1); j++) {
            data.push_back(char_array_3[j]);
        }
    }

    return i ? (4 - i) : 0;
}
