#ifndef CUSTOMMODEL_H
#define CUSTOMMODEL_H

#include "Model.h"


class CustomModel : public Model
{
    public:
        CustomModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased = false);
        virtual ~CustomModel();

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

    protected:
        virtual void InitModel() override;
        virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

    private:
        int GetCustomMaxChannel(const std::string& customModel);
        void InitCustomMatrix(const std::string& customModel);
};

#endif // CUSTOMMODEL_H
