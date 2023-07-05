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
#include "DmxShutterAbility.h"
#include "DmxPanTiltAbility.h"

class Mesh;
class DmxMotor;

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

        int GetNumMotors() const { return NUM_MOTORS; }
        DmxMotor* GetAxis(int num) { return num == 1 ? tilt_motor : pan_motor; }
        DmxMotor* GetPanMotor() { return pan_motor; }
        DmxMotor* GetTiltMotor() { return tilt_motor; }
        void UpdateNodeNames() { update_node_names = true; }
        void UpdateBits() { update_bits = true; }

    protected:
        virtual void InitModel() override;
        void Clear();

        void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool active, const xlColor *c);

        void Draw3DBeam(xlVertexColorAccumulator *vac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open, float beam_offset);

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;
        virtual float GetDefaultBeamWidth() const { return 1.5f; }

        int GetMinChannels();

        float brightness = 100.0f;

    private:
        static const int NUM_MOTORS = 2;

        bool update_node_names = false;
        bool update_bits = false;
        Mesh* base_mesh;
        Mesh* yoke_mesh;
        Mesh* head_mesh;
        DmxMotor* pan_motor;
        DmxMotor* tilt_motor;
        float beam_length;
        float beam_width;
        int beam_orient;
        float beam_y_offset;
};

