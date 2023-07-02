#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxModel.h"
#include "DmxPanTiltAbility.h"
#include "DmxShutterAbility.h"

class Mesh;
class Servo;

class DmxMovingHeadAdv : public DmxModel, public DmxPanTiltAbility, public DmxShutterAbility
{
    public:
    DmxMovingHeadAdv(wxXmlNode* node, const ModelManager& manager, bool zeroBased = false);
        virtual ~DmxMovingHeadAdv();

        virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                          xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                          const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                          bool highlightFirst = false, int highlightpixel = 0,
                                          float *boundingBox = nullptr) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;
        virtual std::list<std::string> CheckModelSettings() override;

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

        void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool active, const xlColor *c);

        void Draw3DBeam(xlVertexColorAccumulator *vac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open, float beam_offset);

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;
        virtual float GetDefaultBeamWidth() const { return 1.5f; }

        float brightness = 100.0f;

    private:
        static const int SUPPORTED_SERVOS = 2;

        bool update_node_names = false;
        bool update_bits = false;
        const int num_servos = 2;
        int num_static = 1;
        int num_motion = 2;
        bool _16bit = true;
        bool show_pivot = false;
        std::vector<Mesh*> static_meshs;
        std::vector<Mesh*> motion_meshs;
        std::vector<Servo*> servos;
        int servo_links[SUPPORTED_SERVOS];
        int mesh_links[SUPPORTED_SERVOS];
        float beam_length;
        float beam_width;
        int beam_orient;
        float beam_y_offset;
};

