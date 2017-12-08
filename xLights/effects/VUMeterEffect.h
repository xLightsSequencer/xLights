#ifndef VUMETEREFFECT_H
#define VUMETEREFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>

#define VUMETER_OFFSET_MIN -100
#define VUMETER_OFFSET_MAX 100

class VUMeterEffect : public RenderableEffect
{
    public:
        VUMeterEffect(int id);
        virtual ~VUMeterEffect();
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        void Render(RenderBuffer &buffer, SequenceElements *elements,
                    int bars, const std::string& type, const std::string& timingtrack, int sensitivity, const std::string& shape, bool slowdownfalls, int startnote, int endnote, int xoffset, int yoffset);
		virtual void SetDefaultParameters(Model *cls) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
		int DecodeType(const std::string&  type);
		int DecodeShape(const std::string& shape);
		void RenderSpectrogramFrame(RenderBuffer &buffer, int bars, std::list<float>& lastvalues, bool slowdownfalls, int startnote, int endnote, int xoffset);
		void RenderVolumeBarsFrame(RenderBuffer &buffer, int bars);
		void RenderWaveformFrame(RenderBuffer &buffer, int bars, int yoffset);
		void RenderTimingEventFrame(RenderBuffer &buffer, int bars, int type, std::string timingtrack, std::list<int> &timingmarks);
        void RenderOnFrame(RenderBuffer &buffer);
        void RenderOnColourFrame(RenderBuffer &buffer);
        void RenderPulseFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, int& lasttimingmark);
        void RenderTimingEventColourFrame(RenderBuffer &buffer, int& colourindex, std::string timingtrack);
        void RenderIntensityWaveFrame(RenderBuffer &buffer, int bars);
        void RenderLevelPulseFrame(RenderBuffer &buffer, int fadeframes, int sensitivity, int& lasttimingmark);
        void RenderLevelBarFrame(RenderBuffer &buffer, int bars, int sensitivity, float& lastbar, int& colourindex);
        void RenderNoteLevelBarFrame(RenderBuffer &buffer, int bars, int sensitivity, float& lastbar, int& colourindex, int startNote, int endNote);
        void RenderLevelShapeFrame(RenderBuffer &buffer, const std::string& shape, float& lastsize, int scale, bool slowdownfalls, int xoffset, int yoffset, int usebars);
        void RenderTimingEventPulseFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, float& lastsize);
        void RenderTimingEventBarFrame(RenderBuffer &buffer, int bars, std::string timingtrack, float& lastbar, int& colourindex, bool all);
        void DrawBox(RenderBuffer& buffer, int startx, int endx, int starty, int endy, xlColor& color1);
        void DrawCircle(RenderBuffer& buffer, int x, int y, float radius, xlColor& color1);
        void DrawStar(RenderBuffer& buffer, int x, int y, float radius, xlColor& color1, int points);
        void DrawDiamond(RenderBuffer& buffer, int centerx, int centery, int size, xlColor& color1);

		void DrawSnowflake(RenderBuffer &buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation = 0);
		void DrawHeart(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
		void DrawTree(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
		void DrawCandycane(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
		void DrawCrucifix(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);
		void DrawPresent(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness = 1);

        void RenderNoteOnFrame(RenderBuffer& buffer, int startNote, int endNote);
        void RenderNoteLevelPulseFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int startNote, int endNote);
        void RenderTimingEventJumpFrame(RenderBuffer &buffer, int fallframes, std::string timingtrack, float& lastval, bool useAudioLevel);
        void RenderLevelPulseColourFrame(RenderBuffer &buffer, int fadeframes, int sensitivity, int& lasttimingmark, int& colourindex);
};

#endif // VUMETEREFFECT_H
