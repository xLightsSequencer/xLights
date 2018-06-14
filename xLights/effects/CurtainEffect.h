#ifndef CURTAINEFFECT_H
#define CURTAINEFFECT_H

#include "RenderableEffect.h"

#include <vector>

#define CURTAIN_SPEED_MIN 0
#define CURTAIN_SPEED_MAX 10

#define CURTAIN_SWAG_MIN 0
#define CURTAIN_SWAG_MAX 10

class CurtainEffect : public RenderableEffect
{
    public:
        CurtainEffect(int id);
        virtual ~CurtainEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
        void DrawCurtain(RenderBuffer &buffer, bool LeftEdge, int xlimit, const std::vector<int> &SwagArray);
        void DrawCurtainVertical(RenderBuffer &buffer, bool topEdge, int ylimit, const std::vector<int> &SwagArray);
};

#endif // CURTAINEFFECT_H
