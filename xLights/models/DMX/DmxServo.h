#ifndef DMXSERVO_H
#define DMXSERVO_H

#include "DmxModel.h"

class DmxImage;
class Servo;

class DmxServo : public DmxModel
{
    public:
        DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxServo();

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        Servo* GetAxis1() { return servo1; }

    protected:
        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        int transparency;
        float brightness;

    private:
        DmxImage* static_image;
        DmxImage* motion_image;
        Servo*    servo1;
};

#endif // DMXSERVO_H
