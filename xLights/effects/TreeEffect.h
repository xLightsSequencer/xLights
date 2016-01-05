#ifndef TREEEFFECT_H
#define TREEEFFECT_H

#include "RenderableEffect.h"


class TreeEffect : public RenderableEffect
{
    public:
        TreeEffect(int id);
        virtual ~TreeEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // TREEEFFECT_H
