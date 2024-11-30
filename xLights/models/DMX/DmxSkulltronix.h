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

#include "DmxModel.h"
#include "DmxColorAbility.h"
#include "DmxPanTiltAbility.h"

class DmxSkulltronix : public DmxModel, public DmxPanTiltAbility
{
    public:
        DmxSkulltronix(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxSkulltronix();

        virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                      xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                      const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                      bool highlightFirst = false, int highlightpixel = 0,
                                      float *boundingBox = nullptr) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;
        void DrawModel(ModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, bool is3d, bool active, const xlColor *c);

        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        int GetEyeBrightnessChannel() {return eye_brightness_channel;}
        int GetPanMinLimit() { return pan_min_limit; }
        int GetPanMaxLimit() { return pan_max_limit; }
        int GetTiltMinLimit() { return tilt_min_limit; }
        int GetTiltMaxLimit() { return tilt_max_limit; }
        int GetNodChannel() {return nod_channel;}
        int GetNodMinLimit() {return nod_min_limit;}
        int GetNodMaxLimit() {return nod_max_limit;}
        int GetJawChannel() {return jaw_channel;}
        int GetJawMinLimit() {return jaw_min_limit;}
        int GetJawMaxLimit() {return jaw_max_limit;}
        int GetEyeUDChannel() {return eye_ud_channel;}
        int GetEyeUDMinLimit() {return eye_ud_min_limit;}
        int GetEyeUDMaxLimit() {return eye_ud_max_limit;}
        int GetEyeLRChannel() {return eye_lr_channel;}
        int GetEyeLRMinLimit() {return eye_lr_min_limit;}
        int GetEyeLRMaxLimit() {return eye_lr_max_limit;}

    protected:
        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        [[nodiscard]] virtual bool ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        int nod_channel;
        int jaw_channel;
        int eye_brightness_channel;
        int eye_ud_channel;
        int eye_lr_channel;
        int nod_orient;
        int nod_deg_of_rot;
        int pan_min_limit;
        int pan_max_limit;
        int tilt_min_limit;
        int tilt_max_limit;
        int nod_min_limit;
        int nod_max_limit;
        int jaw_min_limit;
        int jaw_max_limit;
        int eye_ud_min_limit;
        int eye_ud_max_limit;
        int eye_lr_min_limit;
        int eye_lr_max_limit;

    private:
};

