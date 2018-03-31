#ifndef TREEEFFECT_H
#define TREEEFFECT_H

#include "RenderableEffect.h"


class TreeEffect : public RenderableEffect
{
    public:
        TreeEffect(int id);
        virtual ~TreeEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual bool AppropriateOnNodes() const override { return false; }
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // TREEEFFECT_H
