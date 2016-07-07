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

        virtual void DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *color =  NULL, bool allowSelected = true);
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize);

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid);

        DmxModel(const ModelManager &manager);
        virtual void InitModel() override;

        void InitVMatrix(int firstExportStrand = 0);
        void InitHMatrix();

        void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active);

        std::string dmx_style;
        int dmx_style_val;
        int pan_channel;
        int tilt_channel;
        int red_channel;
        int green_channel;
        int blue_channel;
        int pan_orient;
        int pan_deg_of_rot;
        int tilt_orient;
        int tilt_deg_of_rot;

    private:
};

#endif // DMXMODEL_H
