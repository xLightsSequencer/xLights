#ifndef SINGLESTRANDEFFECT_H
#define SINGLESTRANDEFFECT_H

#include "RenderableEffect.h"

#define SINGLESTRAND_ROTATIONS_MIN 1
#define SINGLESTRAND_ROTATIONS_MAX 200

#define SINGLESTRAND_CHASES_MIN 1
#define SINGLESTRAND_CHASES_MAX 20

#define SINGLESTRAND_COLOURMIX_MIN 1
#define SINGLESTRAND_COLOURMIX_MAX 100

class SingleStrandEffect : public RenderableEffect
{
    public:
        SingleStrandEffect(int id);
        virtual ~SingleStrandEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override { return true; }
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;

    private:    
        void RenderSingleStrandChase(RenderBuffer &buffer,
                                 const std::string &ColorScheme,int Number_Chases,int chaseSize,
                                 const std::string &Chase_Type1,
                                 bool Chase_3dFade1, bool Chase_Group_All,
                                 float chaseSpeed);
        void RenderSingleStrandSkips(RenderBuffer &buffer, Effect *eff, int Skips_BandSize,
                                 int Skips_SkipSize, int Skips_StartPos, const std::string& Skips_Direction, int advances);
        void draw_chase(RenderBuffer &buffer,
                    int x, bool group, int ColorScheme,int Number_Chases,bool autoReverse,int width,
                    int Color_Mix1,bool Chase_Fade3d1,int ChaseDirection, bool mirror);
};

#endif // SINGLESTRANDEFFECT_H
