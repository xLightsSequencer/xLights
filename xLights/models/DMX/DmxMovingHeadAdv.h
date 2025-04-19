#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "DmxMovingHeadComm.h"
#include "DmxDimmerAbility.h"
#include "DmxPanTiltAbility.h"
#include "DmxShutterAbility.h"
#include "DmxMotorBase.h"
#include "DmxMotor.h"
#include <memory>

class Mesh;
class MhFeature;

class DmxMovingHeadAdv : public DmxMovingHeadComm, public DmxDimmerAbility
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
        DmxMotor* GetAxis(int num) { return num == 1 ? tilt_motor.get() : pan_motor.get(); }
        [[nodiscard]] std::vector<std::string> GenerateNodeNames() const override;

        DmxMotorBase* GetPanMotor() const override { return pan_motor.get(); }
        DmxMotorBase* GetTiltMotor() const override { return tilt_motor.get(); }
        void UpdateNodeNames() { update_node_names = true; }
        void UpdateBits() { update_bits = true; }
        float GetBeamYOffset() const { return beam_y_offset; }
        float GetBeamLength() const { return beam_length; }
    
        Mesh* GetBaseMesh() const { return base_mesh; }
        Mesh* GetYokeMesh() const { return yoke_mesh; }
        Mesh* GetHeadMesh() const { return head_mesh; }

        uint32_t GetMHDimmerChannel() const override {return GetDimmerChannel();}

        virtual bool SupportsVisitors() override {return true;}
        void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }

    protected:
        virtual void InitModel() override;
        void Clear();

        void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool active, const xlColor *c);

        void Draw3DBeam(xlVertexColorAccumulator *vac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open, float beam_offset);

        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y, float& min_z, float& max_z) override {
            return true;
        }

        virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;
        virtual float GetDefaultBeamWidth() const { return 1.5f; }

        void CorrectDefaultColorChannels();
        void MapChannelName(wxArrayString& array, int chan, std::string name);

        int GetMinChannels();

        float brightness = 100.0f;

        std::unique_ptr<DmxMotor> pan_motor = nullptr;
        std::unique_ptr<DmxMotor> tilt_motor = nullptr;
    private:
        static const int NUM_MOTORS = 2;

        bool update_node_names = false;
        bool update_bits = false;
        Mesh* base_mesh = nullptr;
        Mesh* yoke_mesh = nullptr;
        Mesh* head_mesh = nullptr;

        wxXmlNode* features_xml_node = nullptr;
        float beam_length;
        float beam_width = 4;
        int beam_orient;
        float beam_y_offset;
        wxString obj_path = "";
        std::vector<std::unique_ptr<MhFeature>> features;
        std::map<std::string, PanTiltState> panTiltStates;
};

