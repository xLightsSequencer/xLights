#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <map>
#include <vector>

#include "ModelScreenLocation.h"
#include "../Color.h"

#include <wx/gdicmn.h>

class DimmingCurve;
class wxXmlNode;
class ModelPreview;
class wxArrayString;
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class ModelScreenLocation;
class ModelManager;

class NodeBaseClass;
typedef std::unique_ptr<NodeBaseClass> NodeBaseClassPtr;

class Model
{
public:
    Model(const ModelManager &manger);
    virtual ~Model();


    std::string name;
    xlColor customColor;
    DimmingCurve *modelDimmingCurve;

    static void ParseFaceInfo(wxXmlNode *fiNode, std::map<std::string, std::map<std::string, std::string> > &faceInfo);
    static void WriteFaceInfo(wxXmlNode *fiNode, const std::map<std::string, std::map<std::string, std::string> > &faceInfo);
    std::map<std::string, std::map<std::string, std::string> > faceInfo;

    virtual const std::vector<std::string> &GetBufferStyles() const { return DEFAULT_BUFFER_STYLES; };
    virtual void GetBufferSize(const std::string &type, const std::string &transform, int &BufferWi, int &BufferHi) const;
    virtual void InitRenderBufferNodes(const std::string &type, const std::string &transform,
                                       std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const;
    const ModelManager &GetModelManager() const {
        return modelManager;
    }

    static const std::vector<std::string> DEFAULT_BUFFER_STYLES;

    virtual bool StrandsZigZagOnString() const { return false;};
    int GetDefaultBufferWi() const {return BufferWi;}
    int GetDefaultBufferHt() const {return BufferHt;}

    void SetProperty(wxString property, wxString value, bool apply = false);
    virtual void AddProperties(wxPropertyGridInterface *grid);
    virtual void AddTypeProperties(wxPropertyGridInterface *grid) {};
    virtual void AddSizeLocationProperties(wxPropertyGridInterface *grid);
    virtual void OnPropertyGridChanging(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {};
    /**
     * Returns a combination of:
     *     0x0001  -  Refresh displays
     *     0x0002  -  Mark settings as "dirty"/"changed"
     *     0x0004  -  Rebuild the property grid
     *     0x0008  -  Rebuild the model list
     *     0x0010  -  Update all model lists
     */
    virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);
protected:
    void AdjustStringProperties(wxPropertyGridInterface *grid, int newNum);
    std::string ComputeStringStartChannel(int x);
    void ApplyTransform(const std::string &transform,
                        std::vector<NodeBaseClassPtr> &Nodes,
                        int &bufferWi, int &bufferHi) const;
    void AdjustForTransform(const std::string &transform,
                            int &bufferWi, int &bufferHi) const;

    int BufferHt,BufferWi;  // size of the default buffer
    std::vector<NodeBaseClassPtr> Nodes;

    const ModelManager &modelManager;

    NodeBaseClass* createNode(int ns, const std::string &StringType, size_t NodesPerString, const std::string &rgbOrder);


    virtual void InitModel();
    virtual int CalcCannelsPerString();
    virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString);


    void SetBufferSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string &rgbOrder);
    void CopyBufCoord2ScreenCoord();

    void SetLineCoord();
    std::string GetNextName();

    int pixelStyle;  //0 - default, 1 - smooth, 2 - circle
    int pixelSize = 2;
    int transparency = 0;
    int blackTransparency = 0;

    int StrobeRate;      // 0=no strobing
    bool zeroBased;
    wxXmlNode* ModelXml;

    virtual const ModelScreenLocation &GetModelScreenLocation() const = 0;
    virtual ModelScreenLocation &GetModelScreenLocation() = 0;

    std::vector<std::string> strandNames;
    std::vector<std::string> nodeNames;
    long parm1;         /* Number of strings in the model or number of arches or canes (except for frames & custom) */
    long parm2;         /* Number of nodes per string in the model or number of segments per arch or cane (except for frames & custom) */
    long parm3;         /* Number of strands per string in the model or number of lights per arch or cane segment (except for frames & custom) */
    bool IsLtoR;         // true=left to right, false=right to left
    std::vector<long> stringStartChan;
    bool isBotToTop;
    std::string StringType; // RGB Nodes, 3 Channel RGB, Single Color Red, Single Color Green, Single Color Blue, Single Color White
    std::string DisplayAs;  // Tree 360, Tree 270, Tree 180, Tree 90, Vert Matrix, Horiz Matrix, Single Line, Arches, Window Frame, Candy Cane
    std::string layout_group;

    unsigned long changeCount;
public:
    void IncrementChangeCount() { ++changeCount;};
    unsigned long GetChangeCount() const { return changeCount; }

    std::string rgbOrder;
    bool SingleNode;     // true for dumb strings and single channel strings
    bool SingleChannel;  // true for traditional single-color strings

    bool Selected=false;
    bool GroupSelected=false;
    std::string ModelStartChannel;
    bool CouldComputeStartChannel;
    bool Overlapping=false;
    void SetFromXml(wxXmlNode* ModelNode, bool zeroBased=false);
    virtual bool ModelRenamed(const std::string &oldName, const std::string &newName);
    size_t GetNodeCount() const;
    int GetChanCount() const;
    int GetChanCountPerNode() const;
    size_t GetCoordCount(size_t nodenum) const;
    int GetNodeStringNumber(size_t nodenum) const;
    void UpdateXmlWithScale();
    void SetOffset(double xPct, double yPct);
    void AddOffset(double xPct, double yPct);
    unsigned int GetLastChannel();
    unsigned int GetFirstChannel();
    int GetNodeNumber(size_t nodenum);
    wxXmlNode* GetModelXml() const;
    int GetNumberFromChannelString(const std::string &sc) const;
    int GetNumberFromChannelString(const std::string &sc, bool &valid) const;
    void DisplayModelOnWindow(ModelPreview* preview, const xlColor *color =  NULL, bool allowSelected = true);
    void DisplayEffectOnWindow(ModelPreview* preview, double pointSize);

    const std::string &GetLayoutGroup() const {return layout_group;}
    void SetLayoutGroup(const std::string &grp);

    void MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY);

    bool HitTest(ModelPreview* preview,int x,int y);
    bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2);
    void SetMinMaxModelScreenCoordinates(ModelPreview* preview);
    void SetMinMaxModelScreenCoordinates(int w, int y);
    const std::string& GetStringType(void) const { return StringType; }
    const std::string& GetDisplayAs(void) const { return DisplayAs; }
    int NodesPerString();
    wxCursor CheckIfOverHandles(int &handle, wxCoord x,wxCoord y);
    wxCursor InitializeLocation(int &handle, wxCoord x,wxCoord y);

    int NodeStartChannel(size_t nodenum) const;
    const std::string &NodeType(size_t nodenum) const;
    virtual int MapToNodeIndex(int strand, int node) const;
    void SetModelStartChan(const std::string &start_channel);
    int ChannelStringToNumber(std::string channel);

    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char *buf);
    xlColor GetNodeColor(size_t nodenum) const;
    wxChar GetChannelColorLetter(wxByte chidx);

    virtual std::string ChannelLayoutHtml();
    //    int FindChannelAt(int x, int y);
    //    wxSize GetChannelCoords(std::vector<std::vector<int>>& chxy, bool shrink); //for pgo RenderFaces
    bool IsCustom(void);
    size_t GetChannelCoords(wxArrayString& choices); //wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3);
    static bool ParseFaceElement(const std::string& str, std::vector<wxPoint>& first_xy);
    //    int FindChannelAtXY(int x, int y, const wxString& model);
    std::string GetNodeXY(const std::string& nodenumstr);
    std::string GetNodeXY(int nodeinx);

    void GetNodeCoords(int nodeidx, std::vector<wxPoint> &pts);

    void SetTop(ModelPreview* preview,int y);
    void SetBottom(ModelPreview* preview,int y);
    void SetLeft(ModelPreview* preview,int x);
    void SetRight(ModelPreview* preview,int x);
    void SetHcenterOffset(float offset);
    void SetVcenterOffset(float offset);

    int GetTop(ModelPreview* preview);
    int GetBottom(ModelPreview* preview);
    int GetLeft(ModelPreview* preview);
    int GetRight(ModelPreview* preview);
    float GetHcenterOffset();
    float GetVcenterOffset();

    bool GetIsLtoR() const {return IsLtoR;}
    bool GetIsBtoT() const {return isBotToTop;}

    virtual int GetStrandLength(int strand) const;

    virtual int GetNumStrands() const;
    std::string GetStrandName(size_t x, bool def = false) const {
        if (x < strandNames.size()) {
            return strandNames[x];
        }
        if (def) {
            return wxString::Format("Strand %d", x + 1).ToStdString();
        }
        return "";
    }
    std::string GetNodeName(size_t x, bool def = false) const {
        if (x < nodeNames.size()) {
            return nodeNames[x];
        }
        if (def) {
            return wxString::Format("Node %d", x + 1).ToStdString();
        }
        return "";
    }

    static std::string StartChanAttrName(int idx)
    {
        return wxString::Format(wxT("String%d"),idx+1).ToStdString();
    }
    // returns true for models that only have 1 string and where parm1 does NOT represent the # of strings
    static bool HasOneString(const std::string& DispAs)
    {
        return (DispAs == "Window Frame" || DispAs == "Custom");
    }
    // true for dumb strings and traditional strings
    static bool HasSingleNode(const std::string& StrType)
    {
        static std::string Nodes(" Nodes");
        if (Nodes.size() > StrType.size()) return false;
        return !std::equal(Nodes.rbegin(), Nodes.rend(), StrType.rbegin());
    }
    // true for traditional strings
    static bool HasSingleChannel(const std::string& StrType)
    {
        return GetNodeChannelCount(StrType) == 1;
    }
    static int GetNodeChannelCount(const std::string & nodeType);

private:
    int maxVertexCount;
};

template <class ScreenLocation>
class ModelWithScreenLocation : public Model {
protected:
    ModelWithScreenLocation(const ModelManager &manager) : Model(manager) {}
    virtual ~ModelWithScreenLocation() {}
    virtual const ModelScreenLocation &GetModelScreenLocation() const  {
        return screenLocation;
    }
    virtual ModelScreenLocation &GetModelScreenLocation() {
        return screenLocation;
    }
protected:
    ScreenLocation screenLocation;
};

#endif // MODEL_H
