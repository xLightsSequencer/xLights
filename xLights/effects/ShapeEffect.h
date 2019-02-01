#ifndef SHAPEEFFECT_H
#define SHAPEEFFECT_H

#include "RenderableEffect.h"

#include "../Color.h"

#define SHAPE_THICKNESS_MIN 1
#define SHAPE_THICKNESS_MAX 100

#define SHAPE_STARTSIZE_MIN 0
#define SHAPE_STARTSIZE_MAX 100

#define SHAPE_CENTREX_MIN 0
#define SHAPE_CENTREX_MAX 100

#define SHAPE_CENTREY_MIN 0 
#define SHAPE_CENTREY_MAX 100

#define SHAPE_LIFETIME_MIN 1
#define SHAPE_LIFETIME_MAX 100

#define SHAPE_GROWTH_MIN -100
#define SHAPE_GROWTH_MAX 100

#define SHAPE_COUNT_MIN 1
#define SHAPE_COUNT_MAX 100

#define SHAPE_ROTATION_MIN 0
#define SHAPE_ROTATION_MAX 360

class ShapeEffect : public RenderableEffect
{
    public:
        ShapeEffect(int id);
        virtual ~ShapeEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetPanelStatus(Model *cls) override;
        virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual bool AppropriateOnNodes() const override { return false; }
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }
#ifdef LINUX
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override { return false; };
#endif
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:

    static int DecodeShape(const std::string& shape);
        void SetPanelTimingTracks() const;
        void Drawcircle(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
        void Drawheart(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
        void Drawstar(RenderBuffer &buffer, int xc, int yc, double radius, int points, xlColor color, int thickness, double rotation = 0) const;
        void Drawpolygon(RenderBuffer &buffer, int xc, int yc, double radius, int sides, xlColor color, int thickness, double rotation = 0) const;
        void Drawsnowflake(RenderBuffer &buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation = 0) const;
        void Drawtree(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
        void Drawcandycane(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
        void Drawcrucifix(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
        void Drawpresent(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const;
        void Drawemoji(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int emoji, wxFontInfo& font) const;
};

#endif // SHAPEEFFECT_H
