#ifndef STARMODEL_H
#define STARMODEL_H

#include "Model.h"


class StarModel : public Model
{
    public:
        StarModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~StarModel();
    
    
        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        int GetStarSize(int starLayer) const {
            return starSizes[starLayer];
        }
        virtual int GetNumStrands() const override;

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
    protected:
        static std::vector<std::string> STAR_BUFFER_STYLES;
        virtual void InitModel() override;

    private:
        std::vector<int> starSizes;
};

#endif // STARMODEL_H
