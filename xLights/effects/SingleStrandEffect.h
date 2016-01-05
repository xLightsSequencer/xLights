#ifndef SINGLESTRANDEFFECT_H
#define SINGLESTRANDEFFECT_H

#include "RenderableEffect.h"


class SingleStrandEffect : public RenderableEffect
{
    public:
        SingleStrandEffect(int id);
        virtual ~SingleStrandEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // SINGLESTRANDEFFECT_H
