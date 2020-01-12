#ifndef DMXSERVO_H
#define DMXSERVO_H

#include "DmxModel.h"

class DmxImage;

class DmxServo : public DmxModel
{
    public:
        DmxServo(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxServo();

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) override;
        virtual int GetChannelValue(int channel) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        void AddTypePropertiesSpecial(wxPropertyGridInterface* grid, bool last = true);
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        int GetMinLimit() { return min_limit; }
        int GetMaxLimit() { return max_limit; }

    protected:
        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        int servo_channel;
        int min_limit;
        int max_limit;
        int range_of_motion;
        int pivot_offset_x;
        int pivot_offset_y;
        int servo_style_val;
        std::string servo_style;
        int transparency;
        float brightness;

    private:
        DmxImage* static_image;
        DmxImage* motion_image;
};

#endif // DMXSERVO_H
