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
#include "DmxMotor.h"
#include "Mesh.h"
#include <memory>
#include <string>
#include <vector>

class MhFeature;

struct PositionZone {
    int pan_min = 0;
    int pan_max = 255;
    int tilt_min = 0;
    int tilt_max = 255;
    int channel = 0;
    uint8_t value = 0;
};
class DmxMovingHeadAdv : public DmxMovingHeadComm
{
    public:
        DmxMovingHeadAdv(const ModelManager& manager);
        virtual ~DmxMovingHeadAdv();

        virtual void DisplayModelOnWindow(IModelPreview* preview, xlGraphicsContext *ctx,
                                          xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                          const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                          bool highlightFirst = false, int highlightpixel = 0,
                                          float *boundingBox = nullptr) override;
        virtual void DisplayEffectOnWindow(IModelPreview* preview, double pointSize) override;

        virtual std::list<std::string> CheckModelSettings() override;

        int GetNumMotors() const { return NUM_MOTORS; }
        DmxMotor* GetAxis(int num) { return num == 1 ? tilt_motor.get() : pan_motor.get(); }
        [[nodiscard]] std::vector<std::string> GenerateNodeNames() const override;

        void UpdateNodeNames() { update_node_names = true; }
        void UpdateBits() { update_bits = true; }
    
        Mesh* CreateBaseMesh(const std::string& name);
        Mesh* CreateYokeMesh(const std::string& name);
        Mesh* CreateHeadMesh(const std::string& name);

        DmxMotor* GetPanMotor() const override { return pan_motor.get(); }
        DmxMotor* GetTiltMotor() const override { return tilt_motor.get(); }

        Mesh* GetBaseMesh() const { return base_mesh.get(); }
        Mesh* GetYokeMesh() const { return yoke_mesh.get(); }
        Mesh* GetHeadMesh() const { return head_mesh.get(); }

        void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }
        void ApplyPositionZones(uint8_t* frameData, uint32_t startChannel) const;
        void AddPositionZone(const PositionZone& zone) { position_zones.push_back(zone); }

    protected:
        virtual void InitModel() override;

        void DrawModel(IModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool active, const xlColor *c);

        void Draw3DBeam(xlVertexColorAccumulator *vac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open, float beam_offset);

        virtual float GetDefaultBeamWidth() const { return 1.5f; }

        void CorrectDefaultColorChannels();
        void MapChannelName(std::vector<std::string>& array, int chan, std::string name);

        int GetMinChannels();

        float brightness = 100.0f;

        std::unique_ptr<DmxMotor> pan_motor = nullptr;
        std::unique_ptr<DmxMotor> tilt_motor = nullptr;
    private:
        static const int NUM_MOTORS = 2;

        bool update_node_names = false;
        bool update_bits = false;

        std::unique_ptr<Mesh> base_mesh = nullptr;
        std::unique_ptr<Mesh> yoke_mesh = nullptr;
        std::unique_ptr<Mesh> head_mesh = nullptr;

        std::string obj_path;
        std::vector<std::unique_ptr<MhFeature>> features;
        std::map<std::string, PanTiltState> panTiltStates;
        std::vector<PositionZone> position_zones;
};

