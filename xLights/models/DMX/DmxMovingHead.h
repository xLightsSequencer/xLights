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
#include "DmxColorAbility.h"
#include "DmxPanTiltAbility.h"
#include "DmxShutterAbility.h"

class DmxMovingHead : public DmxModel, public DmxPanTiltAbility, public DmxShutterAbility
{
    public:
        DmxMovingHead(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxMovingHead();

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float *boundingBox = nullptr) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;
        virtual void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool is3d, bool active, const xlColor *c);
        virtual std::list<std::string> CheckModelSettings() override;
        void EnableFixedChannels(xlColorVector& pixelVector) override;
        [[nodiscard]] std::vector<std::string> GenerateNodeNames() const override;

    protected:
        void Draw3DDMXBaseLeft(xlVertexColorAccumulator &va, const xlColor& c, float pan_angle);
        void Draw3DDMXBaseRight(xlVertexColorAccumulator &va, const xlColor& c, float pan_angle);
        void Draw3DDMXHead(xlVertexColorAccumulator &va, const xlColor& c, float pan_angle, float combinedAngle);
        void Draw3DBeam(xlVertexColorAccumulator *vac, xlColor beam_color, float beam_length_displayed, float pan_angle_raw, float tilt_angle, bool shutter_open);

        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual float GetDefaultBeamWidth() const { return 30; }


        bool hide_body = false;
        bool style_changed;
        std::string dmx_style;
        int dmx_style_val;
        float beam_length;
        float beam_width;

    private:
};
