#ifndef ARCHESMODEL_H
#define ARCHESMODEL_H

#include "Model.h"


class ArchesModel : public Model
{
    public:
        ArchesModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~ArchesModel();
    
        virtual const std::vector<std::string> &GetBufferStyles() const override { return ARCHES_BUFFER_STYLES; };
        virtual void GetBufferSize(const std::string &type, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;
    
    protected:
        static std::vector<std::string> ARCHES_BUFFER_STYLES;
        virtual void InitModel() override;
        virtual int MapToNodeIndex(int strand, int node) const override;
        virtual int GetNumStrands() const override;
        virtual int CalcCannelsPerString() override;

    private:
        void SetArchCoord();
};

#endif // ARCHESMODEL_H
