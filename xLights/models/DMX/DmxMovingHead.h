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
#include "DmxMotor.h"

class DmxMotorBase;

class DmxMovingHead : public DmxMovingHeadComm, public DmxDimmerAbility
{
    public:
        DmxMovingHead(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxMovingHead();

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float *boundingBox = nullptr) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;
        virtual void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool is3d, bool active, const xlColor *c);
        virtual std::list<std::string> CheckModelSettings() override;
        void EnableFixedChannels(xlColorVector& pixelVector) const override;
        [[nodiscard]] std::vector<std::string> GenerateNodeNames() const override;

        [[nodiscard]] DmxMotorBase* GetPanMotor() const override { return pan_motor.get(); }
        [[nodiscard]] DmxMotorBase* GetTiltMotor() const override { return tilt_motor.get(); }

        [[nodiscard]] uint32_t GetMHDimmerChannel() const override {return GetDimmerChannel();}
        [[nodiscard]] std::string const& GetDMXStyle() const { return dmx_style; }
        [[nodiscard]] float GetBeamLength() const { return beam_length; }
        [[nodiscard]] float GetBeamWidth() const { return beam_width; }
        [[nodiscard]] bool GetHideBody() const { return hide_body; }
        [[nodiscard]] virtual bool SupportsVisitors() override { return true; }
        void Accept(BaseObjectVisitor &visitor) const override { return visitor.Visit(*this); }

    protected:
        void Draw3DDMXBaseLeft(xlVertexColorAccumulator &va, const xlColor& c, float pan_angle);
        void Draw3DDMXBaseRight(xlVertexColorAccumulator &va, const xlColor& c, float pan_angle);
        void Draw3DDMXHead(xlVertexColorAccumulator &va, const xlColor& c, float pan_angle, float combinedAngle);
        void Draw3DBeam(xlVertexColorAccumulator *vac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open);

        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual float GetDefaultBeamWidth() const { return 30; }

        std::unique_ptr<DmxMotor> pan_motor = nullptr;
        std::unique_ptr<DmxMotor> tilt_motor = nullptr;
        std::map<std::string, PanTiltState> panTiltStates;

        bool hide_body = false;
        bool style_changed;
        std::string dmx_style;
        int dmx_style_val;
        float beam_length;
        float beam_width;
};
