#ifndef CIRCLEMODEL_H
#define CIRCLEMODEL_H

#include "Model.h"


class CircleModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        CircleModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~CircleModel();

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual bool AllNodesAllocated() const override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;
        virtual bool SupportsExportAsCustom() const override { return true; } 
        virtual bool SupportsWiringView() const override { return true; }

		virtual bool SupportsXlightsModel() override { return true; }
		virtual void ExportXlightsModel() override;
		virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;


    protected:
        virtual void InitModel() override;
        
    private:
        void SetCircleCoord();
        void InitCircle();
    
        int maxSize();

        std::vector<int> circleSizes;
        bool insideOut;
};

#endif // CIRCLEMODEL_H
