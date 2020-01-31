#ifndef DMXSERVO3D_H
#define DMXSERVO3D_H

#include "DmxModel.h"

class Mesh;
class Servo;

class DmxServo3d : public DmxModel
{
    public:
        DmxServo3d(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
        virtual ~DmxServo3d();

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

        Servo* GetAxis(int num) { return num < num_servos ? servos[num] : servos[0]; }
        int GetNumServos() const { return num_servos; }
        int GetNumStatic() const { return num_static; }
        int GetNumMotion() const { return num_motion; }
        void UpdateNodeNames() { update_node_names = true; }
        void UpdateBits() { update_bits = true; }
        bool Is16Bit() const { return _16bit; }

    protected:
        virtual void InitModel() override;
        void Clear();

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        float brightness = 100.0f;

    private:
        static const int SUPPORTED_SERVOS = 24;

        bool update_node_names = false;
        bool update_bits = false;
        int num_servos = 1;
        int num_static = 1;
        int num_motion = 1;
        bool _16bit = true;
        bool show_pivot = false;
        std::vector<Mesh*> static_meshs;
        std::vector<Mesh*> motion_meshs;
        std::vector<Servo*> servos;
        int servo_links[SUPPORTED_SERVOS];
        int mesh_links[SUPPORTED_SERVOS];
};

#endif // DMXSERVO3D_H

