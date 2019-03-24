#ifndef LOREDIT_H
#define LOREDIT_H

#include <vector>
#include "Color.h"
#include <wx/xml/xml.h>

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
    bool pixelChannels;

    std::string GetPalette() const;
    std::string GetxLightsEffect() const;
    std::string GetSettings(std::string& palette) const;

    // Used to rescale a parameter to a broader scale.
    // Assumes the range is different but the translation is direct.
    // If this is not the case then set the source Min/Max to the range that does map to the targetMin/Max and the conversion will
    // clamp original values outside the supported range to the largest practical in the target
    static float Rescale(float original, float sourceMin, float sourceMax, float targetMin, float targetMax);
    static wxString RescaleWithRangeI(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax);
    static wxString RescaleWithRangeF(wxString r, wxString vcName, float sourceMin, float sourceMax, float targetMin, float targetMax, wxString& vc, float targetRealMin, float targetRealMax);
    std::string GetBlend() const;
};

class LOREdit {
    wxXmlDocument& _input_xml;
    int _frequency = 20;

    public:
    LOREdit(wxXmlDocument &input_xml, int frequency);
    virtual ~LOREdit() {};

    std::vector<std::string> GetTimingTracks() const;
    std::vector<std::pair<uint32_t, uint32_t>> GetTimings(const std::string& timingTrackName, int offset = 0) const;
    int GetModelLayers(const std::string& model) const;
    int GetModelChannels(const std::string& model, int& rows, int& cols) const;
    loreditType GetSequencingType(const std::string& model) const;
    std::vector<std::string> GetModelsWithEffects() const;
    std::vector<LOREditEffect> GetTrackEffects(const std::string& model, int layer, int offset = 0) const;
    std::vector<LOREditEffect> GetChannelEffects(const std::string& model, int channel, int offset = 0) const;
    std::vector<LOREditEffect> AddEffects(wxXmlNode* track, bool left, int offset) const;
    static void GetLayers(const std::string& settings, int& ll1, int& ll2);
};

#endif