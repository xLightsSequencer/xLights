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

        Servo* GetAxis(int num) { return num < num_servos ? servos[num] : servos[0]; }
        int GetNumServos() { return num_servos; }

    protected:
        virtual void InitModel() override;
        void Clear();

        virtual void DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active);

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        int transparency;
        float brightness;

    private:
        bool update_node_names;
        int num_servos;
        std::vector<DmxImage*> static_images;
        std::vector<DmxImage*> motion_images;
        std::vector<Servo*> servos;
};

#endif // DMXSERVO_H
