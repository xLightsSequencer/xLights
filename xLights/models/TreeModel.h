#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "MatrixModel.h"


class TreeModel : public MatrixModel
{
    public:
        TreeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~TreeModel();
    
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return true; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
        virtual int NodeRenderOrder() override {return 1;}
        virtual bool IsZScaleable() const override { return true; }

    protected:
        virtual void AddStyleProperties(wxPropertyGridInterface *grid) override;
        virtual void InitModel() override;
    private:
        int treeType;
        long degrees;
        float rotation;
        float spiralRotations;
        float botTopRatio;
        float perspective;
        void SetTreeCoord(long degrees);
};

#endif // TREEMODEL_H
