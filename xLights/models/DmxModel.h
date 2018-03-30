#ifndef DMXMODEL_H
#define DMXMODEL_H

#include "Model.h"


class DmxModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        DmxModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxModel();

        virtual void GetBufferSize(const std::string &type, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;

        virtual void DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *color =  NULL, bool allowSelected = true) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
        virtual int GetNumPhysicalStrings() const override { return 1; }

        int GetEyeBrightnessChannel() {return eye_brightness_channel;}
        int GetRedChannel() {return red_channel;}
        int GetGreenChannel() {return green_channel;}
        int GetBlueChannel() {return blue_channel;}
        int GetPanChannel() {return pan_channel;}
        int GetPanMinLimit() {return pan_min_limit;}
        int GetPanMaxLimit() {return pan_max_limit;}
        int GetTiltChannel() {return tilt_channel;}
        int GetTiltMinLimit() {return tilt_min_limit;}
        int GetTiltMaxLimit() {return tilt_max_limit;}
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
        virtual void AddStyleProperties(wxPropertyGridInterface *grid);

        DmxModel(const ModelManager &manager);
        virtual void InitModel() override;

        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();

        void DrawFloodOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active);
        void DrawSkullModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active);
        void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active);
        int GetChannelValue( int channel );

        void Draw3DDMXBaseLeft(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float& rot_angle);
        void Draw3DDMXBaseRight(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float& rot_angle);
        void Draw3DDMXHead(DrawGLUtils::xlAccumulator &va, const xlColor &c, float &sx, float &sy, float &scale, float &pan_angle, float &tilt_angle);

        bool style_changed;
        std::string dmx_style;
        int dmx_style_val;
        int pan_channel;
        int tilt_channel;
        int nod_channel;
        int jaw_channel;
        int eye_brightness_channel;
        int eye_ud_channel;
        int eye_lr_channel;
        int red_channel;
        int green_channel;
        int blue_channel;
        int pan_orient;
        int pan_deg_of_rot;
        int tilt_orient;
        int tilt_deg_of_rot;
        int nod_orient;
        int nod_deg_of_rot;
        int shutter_channel;
        int shutter_threshold;
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
        float pan_slew_limit;
        float tilt_slew_limit;
        float beam_length;

    private:
};

#endif // DMXMODEL_H
