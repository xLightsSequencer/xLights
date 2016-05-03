#ifndef VIDEOEFFECT_H
#define VIDEOEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
//#include <string>
//#include <list>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/dcmemory.h>

class VideoEffect : public RenderableEffect
{
    public:
        VideoEffect(int id);
        virtual ~VideoEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        void Render(RenderBuffer &buffer,
					const std::string& filename, double starttime, bool keepaspectratio, std::string durationTreatment);
        virtual bool CanBeRandom() override {return false;}

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent)override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect) override;
};

#endif // VIDEOEFFECT_H
