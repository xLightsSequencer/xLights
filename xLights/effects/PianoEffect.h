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
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual int GetColorSupportedCount() const override { return 5; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect) override;

    private:
    
		PianoPanel* _panel;
		void RenderPiano(RenderBuffer &buffer, SequenceElements *elements, const int startmidi, const int endmidi, const bool sharps, const std::string type, std::string timingsource, std::string file, int MIDIAdjustStart, int MIDIAdjustSpeed, int scale, std::string MIDITrack, int xoffset);
		void ReduceChannels(std::list<float>* pdata, int start, int end, bool sharps);
		void DrawTruePiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale, int xoffset);
		void DrawBarsPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale, int xoffset);
		bool IsSharp(float f);
		bool KeyDown(std::list<float>* pdata, int ch);
		std::map<int, std::list<float>> LoadAudacityFile(std::string file, int intervalMS);
		std::map<int, std::list<float>> LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string MIDITrack);
        std::map<int, std::list<float>> LoadTimingTrack(std::string track, int intervalMS);
        int LowerTS(float t, int intervalMS);
		int UpperTS(float t, int intervalMS);
        std::list<std::string> ExtractNotes(std::string& label);
        int ConvertNote(std::string& note);
};

#endif // PIANOEFFECT_H
