#ifndef TREEEFFECT_H
#define TREEEFFECT_H

#include "RenderableEffect.h"


class TreeEffect : public RenderableEffect
{
    public:
        TreeEffect(int id);
        virtual ~TreeEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // TREEEFFECT_H
