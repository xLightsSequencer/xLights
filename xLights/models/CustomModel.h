#ifndef CUSTOMMODEL_H
#define CUSTOMMODEL_H

#include "Model.h"

class CustomModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        CustomModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~CustomModel();

        virtual const std::vector<std::string> &GetBufferStyles() const override;
        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
            std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;

        virtual int GetStrandLength(int strand) const override;
        virtual int MapToNodeIndex(int strand, int node) const override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual std::list<std::string> GetFileReferences() override;
        virtual bool CleanupFileLocations(xLightsFrame* frame) override;

        virtual std::string GetStartLocation() const override { return "n/a"; }

        long GetCustomWidth() const { return parm1;}
        long GetCustomHeight() const { return parm2;}
        long GetCustomDepth() const { return _depth;}
        void SetCustomWidth(long w);
        void SetCustomHeight(long u);
        void SetCustomDepth(long d);
        virtual int NodesPerString() const override;
        virtual int MapPhysicalStringToLogicalString(int string) const override;

        virtual int GetNumPhysicalStrings() const override { return _strings; }

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
        virtual std::string GetNodeName(int x, bool def = false) const override;
        virtual std::list<std::string> CheckModelSettings() override;
        virtual int NodesPerString(int string) const override;

    protected:
        virtual void InitModel() override;
        virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) override;

    private:
        int GetCustomMaxChannel(const std::string& customModel) const;
        void InitCustomMatrix(const std::string& customModel);
        static std::string StartNodeAttrName(int idx)
        {
            return wxString::Format(wxT("String%i"), idx + 1).ToStdString();  // a space between "String" and "%i" breaks the start channels listed in Indiv Start Chans
        }
        std::string ComputeStringStartNode(int x) const;
        int GetCustomNodeStringNumber(int node) const;

        int _depth = 1;
        std::string custom_background;
        int _strings;
        std::vector<int> stringStartNodes;
};

#endif // CUSTOMMODEL_H
