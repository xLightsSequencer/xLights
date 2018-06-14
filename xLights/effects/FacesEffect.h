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
        virtual bool CanBeRandom() override {return false;}
        virtual void SetPanelStatus(Model *cls) override;
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual bool AppropriateOnNodes() const override { return false; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void mouth(RenderBuffer &buffer, int Phoneme,int BufferHt, int BufferWt);
        void coroface(RenderBuffer &buffer, int Phoneme, const std::string& x_y, const std::string& Outline_x_y, const std::string& Eyes_x_y);
        void drawline1(RenderBuffer &buffer, int Phoneme, int x1,int x2,int y1, int y2);
        void drawoutline(RenderBuffer &buffer, int Phoneme, bool outline, const std::string &eyes, int BufferHt,int BufferWi);
        void facesCircle(RenderBuffer &buffer, int Phoneme, int xc,int yc,double radius,int start_degrees, int end_degrees);
        void drawline3 (RenderBuffer &buffer, int Phoneme, int x1,int x2,int y6,int y7);

        void RenderFaces(RenderBuffer &buffer, const std::string &Phoneme, const std::string& eyes, bool face_outline);
        void RenderCoroFacesFromPGO(RenderBuffer &buffer, const std::string& Phoneme, const std::string& eyes, bool face_outline);
        void RenderFaces(RenderBuffer &buffer, SequenceElements *elements, const std::string &faceDefintion,
                         const std::string &Phoneme, const std::string &track, const std::string& eyes, bool face_outline);
        std::string MakeKey(int bufferWi, int bufferHt, std::string dirstr, std::string picture, std::string stf);
};

#endif // FACESEFFECT_H
