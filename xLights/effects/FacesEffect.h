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

#include "RenderableEffect.h"

#include <string>
class SequenceElements;

class FacesEffect : public RenderableEffect
{
public:
    FacesEffect(int id);
    virtual ~FacesEffect();
    virtual bool CanBeRandom() override
    {
        return false;
    }
    virtual void SetPanelStatus(Model* cls) override;
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual std::list<std::string> GetFacesUsed(const SettingsMap& SettingsMap) const override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual wxString GetEffectString() override;

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

private:
    const std::map<std::string, int> eyeBlinkMap;
    void mouth(RenderBuffer& buffer, int Phoneme, int BufferHt, int BufferWt, bool shimmer);
    void drawline1(RenderBuffer& buffer, int Phoneme, int x1, int x2, int y1, int y2, int colorIdx);
    void drawoutline(RenderBuffer& buffer, int Phoneme, bool outline, const std::string& eyes, const std::string& eyeBlinkFreq, int BufferHt, int BufferWi);
    void facesCircle(RenderBuffer& buffer, int Phoneme, int xc, int yc, double radius, int start_degrees, int end_degrees, int colorIdx);
    void drawline3(RenderBuffer& buffer, int Phoneme, int x1, int x2, int y6, int y7, int colorIdx);

    void RenderFaces(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, const std::string& eyeBlinkFreq, bool face_outline, uint8_t alpha, bool suppressShimmer);
    void RenderCoroFacesFromPGO(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, const std::string& eyeBlinkFreq, bool face_outline, uint8_t alpha, bool suppressShimmer);
    void RenderFaces(RenderBuffer& buffer, SequenceElements* elements, const std::string& faceDefintion,
                     const std::string& Phoneme, const std::string& track, const std::string& eyes, const std::string& eyeBlinkFreq, bool face_outline, bool transparentBlack, int transparentBlackLevel, uint8_t alpha, const std::string& outlineState, bool suppressShimmer);
    std::string MakeKey(int bufferWi, int bufferHt, std::string dirstr, std::string picture, std::string stf);
    uint8_t CalculateAlpha(SequenceElements* elements, int leadFrames, bool fade, const std::string& timingTrack, RenderBuffer& buffer);
    bool ShimmerState(RenderBuffer& buffer) const;
    int GetMaxEyeDelay( std::string& eyeBlinkFreq ) const;
};
