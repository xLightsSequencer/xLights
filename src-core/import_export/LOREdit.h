#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>
#include <map>
#include <string>
#include <string_view>
#include <cstdlib>
#include "Color.h"
#include <pugixml.hpp>

class Model;

enum class loreditType
{
    CHANNELS,
    TRACKS,
    NONE
};

struct LOREditEffect
{
    loreditType type;
    uint32_t startMS;
    uint32_t endMS;
    uint16_t startIntensity;
    uint16_t endIntensity;
    xlColor startColour;
    xlColor endColour;
    bool left;
    std::string effectType;
    std::vector<std::string> effectSettings;
    std::vector<std::string> otherSettings;
    bool pixelChannels = false;
    // True when the effect's opposite (left/right) side also holds an effect;
    // LOR's mix setting only means anything in that case.
    bool otherSidePresent = false;
    // The owning track's motion-row region: "none" (whole prop),
    // "rectangle" (sub* are 0-1 fractions, y measured from the top),
    // "custom" or "custom_horizontal_buffer".
    std::string trackType = "none";
    float subx = 0.0f;
    float suby = 0.0f;
    float subw = 1.0f;
    float subh = 1.0f;

    std::string GetPalette() const;
    std::string GetxLightsEffect() const;
    std::string GetSettings(std::string& palette) const;

    // Used to rescale a parameter to a broader scale.
    // Assumes the range is different but the translation is direct.
    // If this is not the case then set the source Min/Max to the range that does map to the targetMin/Max and the conversion will
    // clamp original values outside the supported range to the largest practical in the target
    static float Rescale(float original, float sourceMin, float sourceMax, float targetMin, float targetMax);
    static std::string RescaleWithRangeI(const std::string& r, const std::string& vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, std::string& vc, float targetRealMin, float targetRealMax);
    static std::string RescaleWithRangeF(const std::string& r, const std::string& vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, std::string& vc, float targetRealMin, float targetRealMax);
    std::string GetBlend() const;
    std::string GetLayerSettings() const;
    std::string GetSubBuffer() const;
    // LOR sketches become an embedded SVG drawn by the Pictures effect
    std::string GetSketchSVG() const;
    std::string GetSketchPictureName() const;
    static std::string SafeGetStringParm(const std::vector<std::string>& arr, int param)
    {
        if (param < (int)arr.size())
            return arr[param];
        return "";
    }
    static int SafeGetIntParm(const std::vector<std::string>& arr, int param)
    {
        if (param < (int)arr.size())
            return (int)std::strtol(arr[param].c_str(), nullptr, 10);
        return 0;
    }
    static bool SafeGetBoolParm(const std::vector<std::string>& arr, int param)
    {
        if (param < (int)arr.size())
            return arr[param] == "True";
        return false;
    }
    static std::string ProperCase(const std::string_view& s)
    {
        std::string res;
        bool cap = true;
        for (const auto& it: s) {
            if (it == ' ' || it == '\t') {
                cap = true;
                res += it;
            } else 
            {
                if (cap) {
                    res += std::toupper(it);
                    cap = false;
                } else {
                    res += it;
                }
            }
        }
        return res;
    }
};

class LOREdit {
    pugi::xml_document& _input_xml;
    int _frequency = 20;

    std::vector<LOREditEffect> GetChannelEffectsForNode(int targetRow, int targetCol, int targetColor, pugi::xml_node prop, int offset) const;

    static std::string PropName(pugi::xml_node prop);
    // custom / custom_horizontal_buffer rows cover an arbitrary node subset
    // of the prop, so they are offered as their own "<prop>/<row>" sources
    static bool IsSubRowTrack(pugi::xml_node track);
    // The (track, left side) pairs that become xLights layers for `source` (a
    // prop name, or "<prop>/<row>"), top layer first.
    std::vector<std::pair<pugi::xml_node, bool>> GetSourceLayers(const std::string& source) const;

    public:
    LOREdit(pugi::xml_document &input_xml, int frequency);
    virtual ~LOREdit() {};

    std::vector<std::string> GetTimingTracks() const;
    std::vector<std::pair<uint32_t, uint32_t>> GetTimings(const std::string& timingTrackName, int offset = 0) const;
    int GetModelLayers(const std::string& model) const;
    std::map<int, std::string> GetModelStrands(const std::string& model) const;
    int GetModelChannels(const std::string& model, int& rows, int& cols) const;
    loreditType GetSequencingType(const std::string& model) const;
    // Props with effects, plus a "<prop>/<row>" entry for each custom row that
    // has effects (those rows are not applied when the bare prop is mapped)
    std::vector<std::string> GetModelsWithEffects() const;
    std::vector<std::string> GetNodesWithEffects() const;
    std::vector<LOREditEffect> GetTrackEffects(const std::string& model, int layer, int offset = 0) const;
    std::vector<LOREditEffect> GetChannelEffects(const std::string& model, int channel, Model* m, int offset) const;
    std::vector<LOREditEffect> GetChannelEffects(const std::string& model, int channel, int nodes, int offset) const;
    std::vector<LOREditEffect> GetChannelEffects(const std::string& model, int targetRow, int targetCol, int targetColor, int offset) const;
    std::vector<LOREditEffect> AddEffects(pugi::xml_node track, bool left, int offset) const;
    // base64 image data of the pictures embedded in the file, keyed by the
    // name the picture effects reference
    std::map<std::string, std::string> GetEmbeddedPictures() const;
    // Name an embedded LOR picture is stored under in the xLights sequence
    static std::string EmbeddedPictureName(const std::string& lorName);
    static void GetLayers(const std::string& settings, int& ll1, int& ll2);

    static std::string GetColor(const std::string& settings);

    static bool IsNodeStrandMapping(const std::string& mapping);
    static void setNodeColor(const std::string& color, LOREditEffect& effect);
};
