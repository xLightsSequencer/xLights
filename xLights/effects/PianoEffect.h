#ifndef PIANOEFFECT_H
#define PIANOEFFECT_H

#include "RenderableEffect.h"

#include <list>
class PianoPanel;

#define PIANO_SCALE_MIN 0
#define PIANO_SCALE_MAX 100

class PianoEffect : public RenderableEffect
{
    public:
        PianoEffect(int id);
        virtual ~PianoEffect();
        virtual bool CanBeRandom() override {return false;}
		virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
		static std::vector<float> Parse(wxString& l);
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual int GetColorSupportedCount() const override { return 5; }
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        virtual bool needToAdjustSettings(const std::string& version) override { return true; };
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        void SetPanelTimingTracks();

    private:
    
		PianoPanel* _panel;
		void RenderPiano(RenderBuffer &buffer, SequenceElements *elements, const int startmidi, const int endmidi, const bool sharps, const std::string type, int scale, std::string MIDITrack, int xoffset);
		void ReduceChannels(std::list<float>* pdata, int start, int end, bool sharps);
		void DrawTruePiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale, int xoffset);
		void DrawBarsPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale, int xoffset);
		bool IsSharp(float f);
		bool KeyDown(std::list<float>* pdata, int ch);
        std::map<int, std::list<float>> LoadTimingTrack(std::string track, int intervalMS);
        std::list<std::string> ExtractNotes(std::string& label);
        int ConvertNote(std::string& note);
};

#endif // PIANOEFFECT_H
