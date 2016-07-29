#ifndef POLYLINEMODEL_H
#define POLYLINEMODEL_H

#include "Model.h"


class PolyLineModel : public ModelWithScreenLocation<PolyPointScreenLocation>
{
    public:
        PolyLineModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);

        PolyLineModel(int lights, const Model &base, int strand, int node = -1);
        PolyLineModel(const ModelManager &manager);
        virtual ~PolyLineModel();

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        int GetPolyLineSize(int polyLineLayer) const {
            return polyLineSizes[polyLineLayer];
        }
        virtual int GetNumStrands() const override;

        void InitPolyLine();
        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual void InsertHandle(int after_handle) override;
        virtual void DeleteHandle(int handle) override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        virtual void InitModel() override;
        void SavePolyLineSizes();
        int num_segments;
        int longest_segment;
        struct xlPolyPoint {
            float x;
            float y;
            float length;
            mutable glm::mat3 *matrix;
        };
        float total_length;

    private:
        std::vector<int> polyLineSizes;
};

#endif // SINGLELINEMODEL_H
