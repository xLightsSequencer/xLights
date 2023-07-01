#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/colour.h>
#include <wx/gdicmn.h>
#include <wx/xml/xml.h>

#include <map>
#include <cmath>

struct VixenTiming
{
    VixenTiming(float s, float e, const std::string& l) : start(s), end(e), label(l) { }
    float start;
    float end;
    std::string label;
};

struct VixenColor
{
    VixenColor(wxColor c, float p) : color(c), position(p) { }
    wxColor color;
    float position;

    bool operator<(const VixenColor& a) const
    {
        return position < a.position;
    }
};

class VixenEffect
{
public:
    VixenEffect(float s, float e, const std::string& esid) :
        start(s), end(e), effectSettingsId(esid), type("") {}

    std::string GetPalette() const;
    std::string GetSettings() const;
    std::string GetXLightsType() const;

    float start;
    float end;
    std::string effectSettingsId;
    std::string type;
    std::map<std::string, std::string> settings;
    std::vector<std::vector<VixenColor>> palette;
    std::vector<wxRealPoint> levelCurve;

    static constexpr const std::uint32_t KNOWN_COLOR[] = {
        0x00000000, /* 000 - Empty */
        0xFFD4D0C8, /* 001 - ActiveBorder */
        0xFF0054E3, /* 002 - ActiveCaption */
        0xFFFFFFFF, /* 003 - ActiveCaptionText */
        0xFF808080, /* 004 - AppWorkspace */
        0xFFECE9D8, /* 005 - Control */
        0xFFACA899, /* 006 - ControlDark */
        0xFF716F64, /* 007 - ControlDarkDark */
        0xFFF1EFE2, /* 008 - ControlLight */
        0xFFFFFFFF, /* 009 - ControlLightLight */
        0xFF000000, /* 010 - ControlText */
        0xFF004E98, /* 011 - Desktop */
        0xFFACA899, /* 012 - GrayText */
        0xFF316AC5, /* 013 - Highlight */
        0xFFFFFFFF, /* 014 - HighlightText */
        0xFF000080, /* 015 - HotTrack */
        0xFFD4D0C8, /* 016 - InactiveBorder */
        0xFF7A96DF, /* 017 - InactiveCaption */
        0xFFD8E4F8, /* 018 - InactiveCaptionText */
        0xFFFFFFE1, /* 019 - Info */
        0xFF000000, /* 020 - InfoText */
        0xFFFFFFFF, /* 021 - Menu */
        0xFF000000, /* 022 - MenuText */
        0xFFD4D0C8, /* 023 - ScrollBar */
        0xFFFFFFFF, /* 024 - Window */
        0xFF000000, /* 025 - WindowFrame */
        0xFF000000, /* 026 - WindowText */
        0x00FFFFFF, /* 027 - Transparent */
        0xFFF0F8FF, /* 028 - AliceBlue */
        0xFFFAEBD7, /* 029 - AntiqueWhite */
        0xFF00FFFF, /* 030 - Aqua */
        0xFF7FFFD4, /* 031 - Aquamarine */
        0xFFF0FFFF, /* 032 - Azure */
        0xFFF5F5DC, /* 033 - Beige */
        0xFFFFE4C4, /* 034 - Bisque */
        0xFF000000, /* 035 - Black */
        0xFFFFEBCD, /* 036 - BlanchedAlmond */
        0xFF0000FF, /* 037 - Blue */
        0xFF8A2BE2, /* 038 - BlueViolet */
        0xFFA52A2A, /* 039 - Brown */
        0xFFDEB887, /* 040 - BurlyWood */
        0xFF5F9EA0, /* 041 - CadetBlue */
        0xFF7FFF00, /* 042 - Chartreuse */
        0xFFD2691E, /* 043 - Chocolate */
        0xFFFF7F50, /* 044 - Coral */
        0xFF6495ED, /* 045 - CornflowerBlue */
        0xFFFFF8DC, /* 046 - Cornsilk */
        0xFFDC143C, /* 047 - Crimson */
        0xFF00FFFF, /* 048 - Cyan */
        0xFF00008B, /* 049 - DarkBlue */
        0xFF008B8B, /* 050 - DarkCyan */
        0xFFB8860B, /* 051 - DarkGoldenrod */
        0xFFA9A9A9, /* 052 - DarkGray */
        0xFF006400, /* 053 - DarkGreen */
        0xFFBDB76B, /* 054 - DarkKhaki */
        0xFF8B008B, /* 055 - DarkMagenta */
        0xFF556B2F, /* 056 - DarkOliveGreen */
        0xFFFF8C00, /* 057 - DarkOrange */
        0xFF9932CC, /* 058 - DarkOrchid */
        0xFF8B0000, /* 059 - DarkRed */
        0xFFE9967A, /* 060 - DarkSalmon */
        0xFF8FBC8B, /* 061 - DarkSeaGreen */
        0xFF483D8B, /* 062 - DarkSlateBlue */
        0xFF2F4F4F, /* 063 - DarkSlateGray */
        0xFF00CED1, /* 064 - DarkTurquoise */
        0xFF9400D3, /* 065 - DarkViolet */
        0xFFFF1493, /* 066 - DeepPink */
        0xFF00BFFF, /* 067 - DeepSkyBlue */
        0xFF696969, /* 068 - DimGray */
        0xFF1E90FF, /* 069 - DodgerBlue */
        0xFFB22222, /* 070 - Firebrick */
        0xFFFFFAF0, /* 071 - FloralWhite */
        0xFF228B22, /* 072 - ForestGreen */
        0xFFFF00FF, /* 073 - Fuchsia */
        0xFFDCDCDC, /* 074 - Gainsboro */
        0xFFF8F8FF, /* 075 - GhostWhite */
        0xFFFFD700, /* 076 - Gold */
        0xFFDAA520, /* 077 - Goldenrod */
        0xFF808080, /* 078 - Gray */
        0xFF008000, /* 079 - Green */
        0xFFADFF2F, /* 080 - GreenYellow */
        0xFFF0FFF0, /* 081 - Honeydew */
        0xFFFF69B4, /* 082 - HotPink */
        0xFFCD5C5C, /* 083 - IndianRed */
        0xFF4B0082, /* 084 - Indigo */
        0xFFFFFFF0, /* 085 - Ivory */
        0xFFF0E68C, /* 086 - Khaki */
        0xFFE6E6FA, /* 087 - Lavender */
        0xFFFFF0F5, /* 088 - LavenderBlush */
        0xFF7CFC00, /* 089 - LawnGreen */
        0xFFFFFACD, /* 090 - LemonChiffon */
        0xFFADD8E6, /* 091 - LightBlue */
        0xFFF08080, /* 092 - LightCoral */
        0xFFE0FFFF, /* 093 - LightCyan */
        0xFFFAFAD2, /* 094 - LightGoldenrodYellow */
        0xFFD3D3D3, /* 095 - LightGray */
        0xFF90EE90, /* 096 - LightGreen */
        0xFFFFB6C1, /* 097 - LightPink */
        0xFFFFA07A, /* 098 - LightSalmon */
        0xFF20B2AA, /* 099 - LightSeaGreen */
        0xFF87CEFA, /* 100 - LightSkyBlue */
        0xFF778899, /* 101 - LightSlateGray */
        0xFFB0C4DE, /* 102 - LightSteelBlue */
        0xFFFFFFE0, /* 103 - LightYellow */
        0xFF00FF00, /* 104 - Lime */
        0xFF32CD32, /* 105 - LimeGreen */
        0xFFFAF0E6, /* 106 - Linen */
        0xFFFF00FF, /* 107 - Magenta */
        0xFF800000, /* 108 - Maroon */
        0xFF66CDAA, /* 109 - MediumAquamarine */
        0xFF0000CD, /* 110 - MediumBlue */
        0xFFBA55D3, /* 111 - MediumOrchid */
        0xFF9370DB, /* 112 - MediumPurple */
        0xFF3CB371, /* 113 - MediumSeaGreen */
        0xFF7B68EE, /* 114 - MediumSlateBlue */
        0xFF00FA9A, /* 115 - MediumSpringGreen */
        0xFF48D1CC, /* 116 - MediumTurquoise */
        0xFFC71585, /* 117 - MediumVioletRed */
        0xFF191970, /* 118 - MidnightBlue */
        0xFFF5FFFA, /* 119 - MintCream */
        0xFFFFE4E1, /* 120 - MistyRose */
        0xFFFFE4B5, /* 121 - Moccasin */
        0xFFFFDEAD, /* 122 - NavajoWhite */
        0xFF000080, /* 123 - Navy */
        0xFFFDF5E6, /* 124 - OldLace */
        0xFF808000, /* 125 - Olive */
        0xFF6B8E23, /* 126 - OliveDrab */
        0xFFFFA500, /* 127 - Orange */
        0xFFFF4500, /* 128 - OrangeRed */
        0xFFDA70D6, /* 129 - Orchid */
        0xFFEEE8AA, /* 130 - PaleGoldenrod */
        0xFF98FB98, /* 131 - PaleGreen */
        0xFFAFEEEE, /* 132 - PaleTurquoise */
        0xFFDB7093, /* 133 - PaleVioletRed */
        0xFFFFEFD5, /* 134 - PapayaWhip */
        0xFFFFDAB9, /* 135 - PeachPuff */
        0xFFCD853F, /* 136 - Peru */
        0xFFFFC0CB, /* 137 - Pink */
        0xFFDDA0DD, /* 138 - Plum */
        0xFFB0E0E6, /* 139 - PowderBlue */
        0xFF800080, /* 140 - Purple */
        0xFFFF0000, /* 141 - Red */
        0xFFBC8F8F, /* 142 - RosyBrown */
        0xFF4169E1, /* 143 - RoyalBlue */
        0xFF8B4513, /* 144 - SaddleBrown */
        0xFFFA8072, /* 145 - Salmon */
        0xFFF4A460, /* 146 - SandyBrown */
        0xFF2E8B57, /* 147 - SeaGreen */
        0xFFFFF5EE, /* 148 - SeaShell */
        0xFFA0522D, /* 149 - Sienna */
        0xFFC0C0C0, /* 150 - Silver */
        0xFF87CEEB, /* 151 - SkyBlue */
        0xFF6A5ACD, /* 152 - SlateBlue */
        0xFF708090, /* 153 - SlateGray */
        0xFFFFFAFA, /* 154 - Snow */
        0xFF00FF7F, /* 155 - SpringGreen */
        0xFF4682B4, /* 156 - SteelBlue */
        0xFFD2B48C, /* 157 - Tan */
        0xFF008080, /* 158 - Teal */
        0xFFD8BFD8, /* 159 - Thistle */
        0xFFFF6347, /* 160 - Tomato */
        0xFF40E0D0, /* 161 - Turquoise */
        0xFFEE82EE, /* 162 - Violet */
        0xFFF5DEB3, /* 163 - Wheat */
        0xFFFFFFFF, /* 164 - White */
        0xFFF5F5F5, /* 165 - WhiteSmoke */
        0xFFFFFF00, /* 166 - Yellow */
        0xFF9ACD32, /* 167 - YellowGreen */
        0xFFECE9D8, /* 168 - ButtonFace */
        0xFFFFFFFF, /* 169 - ButtonHighlight */
        0xFFACA899, /* 170 - ButtonShadow */
        0xFF3D95FF, /* 171 - GradientActiveCaption */
        0xFF9DB9EB, /* 172 - GradientInactiveCaption */
        0xFFECE9D8, /* 173 - MenuBar */
        0xFF316AC5, /* 174 - MenuHighlight */
    };
};

class Vixen3 {

    wxXmlDocument _doc;
    std::string _filename;
    std::string _systemFile;
    std::map<std::string, std::list<VixenTiming>> _timingData;
    std::map<std::string, std::string> _timingType;
    std::map<std::string, std::list<VixenEffect>> _effectData;
    bool _systemFound = false;

    void ProcessNode(wxXmlNode* n, std::map<std::string, std::string>& models);
    std::vector<VixenColor> ProcessColorData(wxXmlNode* n);

    public:

    Vixen3(const std::string& filename, const std::string& system = "");
    virtual ~Vixen3() {}

    inline static long ConvertTiming(float time, int frame)
    {
        return std::round(time * 1000.0 / (float)frame) * frame;
    }
    const std::string& GetFilename() const { return _filename; }
    bool IsSystemFound() const { return _systemFound; }

    std::list<std::string> GetTimings() const;
    std::list<VixenTiming> GetTimings(const std::string& timing) const;
    std::string GetTimingType(const std::string& timing) const;
    std::list<VixenTiming> GetRelatedTiming(const std::string& timing, const std::string& type) const;

    std::list<std::pair<std::string, int>> GetModelsWithEffects() const;
    std::list<VixenEffect> GetEffects(const std::string& model) const;
};
