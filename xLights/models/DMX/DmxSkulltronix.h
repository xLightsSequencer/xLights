#ifndef DMXSKULLTRONIX_H
#define DMXSKULLTRONIX_H

#include "DmxModel.h"
#include "DmxColorAbility.h"
#include "DmxPanTiltAbility.h"

class DmxSkulltronix : public DmxModel, public DmxColorAbility, public DmxPanTiltAbility
{
    public:
        DmxSkulltronix(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxSkulltronix();

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) override;
        virtual int GetChannelValue(int channel) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool HasColorAbility() override { return true; }

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
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

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

#endif // DMXSKULLTRONIX_H
