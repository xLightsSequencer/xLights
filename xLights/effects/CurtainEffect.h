#ifndef CURTAINEFFECT_H
#define CURTAINEFFECT_H

#include "RenderableEffect.h"

#include <vector>

class CurtainEffect : public RenderableEffect
{
    public:
        CurtainEffect(int id);
        virtual ~CurtainEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
        void DrawCurtain(RenderBuffer &buffer, bool LeftEdge, int xlimit, const std::vector<int> &SwagArray);
        void DrawCurtainVertical(RenderBuffer &buffer, bool topEdge, int ylimit, const std::vector<int> &SwagArray);
};

#endif // CURTAINEFFECT_H
