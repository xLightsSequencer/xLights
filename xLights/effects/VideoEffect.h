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
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
					const std::string& filename, double starttime, bool keepaspectratio, std::string durationTreatment);
        virtual bool CanBeRandom() {return false;}

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
        virtual bool needToAdjustSettings(const std::string& version) { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect);
};

#endif // VIDEOEFFECT_H
