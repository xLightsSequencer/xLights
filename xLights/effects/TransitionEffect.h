#ifndef TRANSITIONEFFECT_H
#define TRANSITIONEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>

class TransitionEffect : public RenderableEffect
{
    public:
        TransitionEffect(int id);
        virtual ~TransitionEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
                    bool mode, const std::string& type, int adjust, bool reverse);
        virtual bool CanRenderOnBackgroundThread();
		bool isLeft(wxPoint a, float slope, wxPoint test);
	protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
		int DecodeType(std::string type);
		void RenderWipe(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust);
		void RenderClockWise(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderFromMiddle(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderSquareExplode(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderCircleExplode(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderBlinds(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderBlend(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderSlideChecks(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
		void RenderSlideBars(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse);
};

#endif // VUMETEREFFECT_H
