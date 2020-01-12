#ifndef DMXFLOODAREA_H
#define DMXFLOODAREA_H

#include "DmxFloodlight.h"
#include "DmxColorAbility.h"
#include "DmxShutterAbility.h"

class DmxFloodArea : public DmxFloodlight
{
    public:
        DmxFloodArea(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxFloodArea();

    protected:
        virtual void InitModel() override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

    private:
};

#endif // DMXFLOODAREA_H
