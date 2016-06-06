#ifndef CUSTOMMODEL_H
#define CUSTOMMODEL_H

#include "Model.h"

class CustomModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        CustomModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~CustomModel();

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        long GetCustomWidth() const { return parm1;}
        long GetCustomHeight() const { return parm2;}
        void SetCustomWidth(long w);
        void SetCustomHeight(long u);

        std::string GetCustomData() const;
        void SetCustomData(const std::string &data);

        std::string GetCustomBackground() const {return custom_background;}
        long GetCustomLightness() const;
        void SetCustomLightness(long lightness);

        virtual std::string ChannelLayoutHtml() override;

    protected:
        virtual void InitModel() override;
        virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

    private:
        int GetCustomMaxChannel(const std::string& customModel);
        void InitCustomMatrix(const std::string& customModel);

        std::string custom_background;
};

#endif // CUSTOMMODEL_H
