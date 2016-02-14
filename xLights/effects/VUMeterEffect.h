#ifndef VUMETEREFFECT_H
#define VUMETEREFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>

class VUMeterEffect : public RenderableEffect
{
    public:
        VUMeterEffect(int id);
        virtual ~VUMeterEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
                    int bars, const std::string& type, const std::string& timingtrack);
        virtual bool CanRenderOnBackgroundThread();
		virtual void SetDefaultParameters(Model *cls);
	protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
		int DecodeType(std::string type);
		void RenderSpectrogramFrame(RenderBuffer &buffer, int bars, std::list<float>& lastvalues, bool gravity);
		void RenderVolumeBarsFrame(RenderBuffer &buffer, int bars);
		void RenderWaveformFrame(RenderBuffer &buffer, int bars);
		void RenderTimingEventFrame(RenderBuffer &buffer, int bars, int type, std::string timingtrack, std::list<int> &timingmarks);
		void RenderOnFrame(RenderBuffer &buffer);
		void RenderPulseFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, int& lasttimingmark);
		void RenderIntensityWaveFrame(RenderBuffer &buffer, int bars);
};

#endif // VUMETEREFFECT_H
