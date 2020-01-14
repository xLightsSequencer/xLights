#ifndef DMXSERVO3AXIS_H
#define DMXSERVO3AXIS_H

#include "DmxModel.h"
#include "Mesh.h"
class Servo;

class DmxServo3Axis : public DmxModel
{
public:
    DmxServo3Axis(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
    virtual ~DmxServo3Axis();

    virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) override;
    virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) override;

    virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
    virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

    Servo* GetAxis1() { return servo1; }
    Servo* GetAxis2() { return servo2; }
    Servo* GetAxis3() { return servo3; }

protected:
    virtual void InitModel() override;

    virtual void ExportXlightsModel() override;
    virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

    float brightness;

private:
    Mesh* static_mesh;
    Mesh* motion_mesh1;
    Mesh* motion_mesh2;
    Mesh* motion_mesh3;
    Servo* servo1;
    Servo* servo2;
    Servo* servo3;
};

#endif // DMXSERVO3AXIS_H
