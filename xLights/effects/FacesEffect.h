#ifndef FACESEFFECT_H
#define FACESEFFECT_H

#include "RenderableEffect.h"

#include <string>
class SequenceElements;

class FacesEffect : public RenderableEffect
{
public:
    FacesEffect(int id);
    virtual ~FacesEffect();
    virtual bool CanBeRandom() override { return false; }
    virtual void SetPanelStatus(Model* cls) override;
    virtual void SetDefaultParameters() override;
    virtual void Render(Effect* effect, SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
    virtual bool AppropriateOnNodes() const override { return false; }
    virtual std::list<std::string> GetFacesUsed(const SettingsMap& SettingsMap) const override;
protected:
    virtual wxPanel* CreatePanel(wxWindow* parent) override;
private:
    void mouth(RenderBuffer& buffer, int Phoneme, int BufferHt, int BufferWt, bool shimmer);
    void drawline1(RenderBuffer& buffer, int Phoneme, int x1, int x2, int y1, int y2, int colorIdx);
    void drawoutline(RenderBuffer& buffer, int Phoneme, bool outline, const std::string& eyes, int BufferHt, int BufferWi);
    void facesCircle(RenderBuffer& buffer, int Phoneme, int xc, int yc, double radius, int start_degrees, int end_degrees, int colorIdx);
    void drawline3(RenderBuffer& buffer, int Phoneme, int x1, int x2, int y6, int y7, int colorIdx);

    void RenderFaces(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, bool face_outline, bool suppressIfNotSinging);
    void RenderCoroFacesFromPGO(RenderBuffer& buffer, const std::string& Phoneme, const std::string& eyes, bool face_outline, bool suppressIfNotSinging);
    void RenderFaces(RenderBuffer& buffer, SequenceElements* elements, const std::string& faceDefintion,
        const std::string& Phoneme, const std::string& track, const std::string& eyes, bool face_outline, bool transparentBlack, int transparentBlackLevel, bool suppressIfNotSinging);
    std::string MakeKey(int bufferWi, int bufferHt, std::string dirstr, std::string picture, std::string stf);
};

#endif // FACESEFFECT_H
