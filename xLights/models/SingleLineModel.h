#ifndef SINGLELINEMODEL_H
#define SINGLELINEMODEL_H

#include "Model.h"


class SingleLineModel : public Model
{
    public:
        SingleLineModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
    
        SingleLineModel(int lights, const Model &base, int strand, int node = -1);
        SingleLineModel();
        virtual ~SingleLineModel();

        void InitLine();
    
        void Reset(int lights, const Model &base, int strand, int node = -1, bool forceDirection = false);
        virtual const std::vector<std::string> &GetBufferStyles() const;

    protected:
        static std::vector<std::string> LINE_BUFFER_STYLES;
        virtual void InitModel() override;

    private:
};

#endif // SINGLELINEMODEL_H
