#ifndef DMXMOVINGHEAD3D_H
#define DMXMOVINGHEAD3D_H

#include "DmxMovingHead.h"

class DmxMovingHead3D : public DmxMovingHead
{
    public:
        DmxMovingHead3D(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxMovingHead3D();

    protected:
        virtual void InitModel() override;

        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

        void Draw3DDMXBaseLeft(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float& rot_angle);
        void Draw3DDMXBaseRight(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float& rot_angle);
        void Draw3DDMXHead(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float &tilt_angle);
        void Draw3DDMXBaseLeft(DrawGLUtils::xl3Accumulator &va, const xlColor &c, float &sx, float &sy, float &sz, float &scale, float &pan_angle, float& rot_angle);
        void Draw3DDMXBaseRight(DrawGLUtils::xl3Accumulator &va, const xlColor &c, float &sx, float &sy, float &sz, float &scale, float &pan_angle, float& rot_angle);
        void Draw3DDMXHead(DrawGLUtils::xl3Accumulator &va, const xlColor &c, float &sx, float &sy, float &sz, float &scale, float &pan_angle, float &tilt_angle);

    private:
};

#endif // DMXMOVINGHEAD3D_H
