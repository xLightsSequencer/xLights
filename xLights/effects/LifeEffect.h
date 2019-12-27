#ifndef LIFEEFFECT_H
#define LIFEEFFECT_H

#include "RenderableEffect.h"


class LifeEffect : public RenderableEffect
{
    public:
        LifeEffect(int id);
        virtual ~LifeEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool AppropriateOnNodes() const override { return false; }
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // LIFEEFFECT_H
