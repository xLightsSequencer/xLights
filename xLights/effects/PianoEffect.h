#ifndef PIANOEFFECT_H
#define PIANOEFFECT_H

#include "RenderableEffect.h"

#include <string>
#include <list>

class PianoEffect : public RenderableEffect
{
    public:
        PianoEffect(int id);
        virtual ~PianoEffect();
        virtual bool CanBeRandom() {return false;}
		virtual bool CanRenderOnBackgroundThread() { return true; }
		virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
    
		void RenderPiano(RenderBuffer &buffer, const int startmidi, const int endmidi, const bool sharps, const std::string type);
		void ReduceChannels(std::list<float>* pdata, int start, int end, bool sharps);
		void DrawTruePiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end);
		void DrawBarsPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end);
		bool IsSharp(float f);
		bool KeyDown(std::list<float>* pdata, int ch);
};

#endif // PIANOEFFECT_H
