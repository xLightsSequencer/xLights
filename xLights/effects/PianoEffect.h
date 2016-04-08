#ifndef PIANOEFFECT_H
#define PIANOEFFECT_H

#include "RenderableEffect.h"

#include <string>
#include <list>
class PianoPanel;

class PianoEffect : public RenderableEffect
{
    public:
        PianoEffect(int id);
        virtual ~PianoEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
		virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
		static std::vector<float> Parse(wxString& l);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect);

    private:
    
		PianoPanel* _panel;
		void RenderPiano(RenderBuffer &buffer, const int startmidi, const int endmidi, const bool sharps, const std::string type, std::string timingsource, std::string file, int MIDIAdjustStart, int MIDIAdjustSpeed, int scale, std::string MIDITrack);
		void ReduceChannels(std::list<float>* pdata, int start, int end, bool sharps);
		void DrawTruePiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale);
		void DrawBarsPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale);
		bool IsSharp(float f);
		bool KeyDown(std::list<float>* pdata, int ch);
		std::map<int, std::list<float>> LoadAudacityFile(std::string file, int intervalMS);
		std::map<int, std::list<float>> LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string MIDITrack);
		int LowerTS(float t, int intervalMS);
		int UpperTS(float t, int intervalMS);
};

#endif // PIANOEFFECT_H
