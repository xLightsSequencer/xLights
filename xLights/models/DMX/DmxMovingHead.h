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

class DmxMovingHead : public DmxModel, public DmxColorAbility, public DmxPanTiltAbility, public DmxShutterAbility
{
    public:
        DmxMovingHead(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxMovingHead();

        virtual bool HasColorAbility() override { return true; }

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        void DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor* c, float& sx, float& sy, float& sz, bool active, bool is_3d);
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

        void Draw3DDMXBaseLeft(DrawGLUtils::xlAccumulator& va, const xlColor& c, float& sx, float& sy, float& scale, float& pan_angle, float& rot_angle);
        void Draw3DDMXBaseRight(DrawGLUtils::xlAccumulator& va, const xlColor& c, float& sx, float& sy, float& scale, float& pan_angle, float& rot_angle);
        void Draw3DDMXHead(DrawGLUtils::xlAccumulator& va, const xlColor& c, float& sx, float& sy, float& scale, float& pan_angle, float& tilt_angle);

        bool hide_body = false;
        bool style_changed;
        std::string dmx_style;
        int dmx_style_val;
        float beam_length;

    private:
};
