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
        virtual int NodesPerString() override;

        virtual int GetNumPhysicalStrings() const override { return 1; }

        std::string GetCustomData() const;
        void SetCustomData(const std::string &data);

        std::string GetCustomBackground() const {return custom_background;}
        void SetCustomBackground(std::string background);
        long GetCustomLightness() const;
        void SetCustomLightness(long lightness);

        virtual bool SupportsXlightsModel() override {return true;}
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return true; }
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;
        void ImportLORModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y);
        virtual void ExportXlightsModel() override;

        virtual std::string ChannelLayoutHtml(OutputManager* outputManager) override;
        virtual std::string GetNodeName(size_t x, bool def = false) const override;
        virtual std::list<std::string> CheckModelSettings() override;

    protected:
        virtual void InitModel() override;
        virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

    private:
        int GetCustomMaxChannel(const std::string& customModel) const;
        void InitCustomMatrix(const std::string& customModel);

        std::string custom_background;
};

#endif // CUSTOMMODEL_H
