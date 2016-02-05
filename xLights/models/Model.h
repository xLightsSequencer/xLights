#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <map>
#include <vector>

#include "Node.h"
#include "../Color.h"

#include <wx/gdicmn.h>

class DimmingCurve;
class wxXmlNode;
class NetInfoClass;
class ModelPreview;
class wxArrayString;

class Model
{
public:
    Model();
    virtual ~Model();

    
    
    std::string name;
    int BufferHt,BufferWi;  // size of the buffer
    xlColor customColor;
    std::vector<NodeBaseClassPtr> Nodes;
    
    std::map<std::string, std::map<std::string, std::string> > faceInfo;
    DimmingCurve *modelDimmingCurve;
    
    void InitRenderBufferNodes(int type, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const;
    bool IsMyDisplay() { return isMyDisplay;}
protected:
    static NodeBaseClass* createNode(int ns, const std::string &StringType, size_t NodesPerString, const std::string &rgbOrder);
    
    
    virtual void InitModel();
    virtual int CalcCannelsPerString();
    virtual void SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString);

    void InitVMatrix(int firstExportStrand = 0);
    void InitHMatrix();
    void InitFrame();
    void InitWreath();
    void InitSphere();
    
    void SetBufferSize(int NewHt, int NewWi);
    void SetRenderSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string &rgbOrder);
    void CopyBufCoord2ScreenCoord();
    
    void SetLineCoord();
    double toRadians(long degrees);
    long toDegrees(double radians);
    std::string GetNextName();
    
    int pixelStyle;  //0 - default, 1 - smooth, 2 - circle
    int pixelSize = 2;
    int transparency = 0;
    int blackTransparency = 0;
    
    int FrameWidth;
    bool modelv2;
    int StrobeRate;      // 0=no strobing
    double offsetXpct,offsetYpct;
    bool singleScale;
    double PreviewScaleX, PreviewScaleY;
    int PreviewRotation;
    long ModelVersion;
    bool zeroBased;
    bool isMyDisplay;
    
    int mMinScreenX;
    int mMinScreenY;
    int mMaxScreenX;
    int mMaxScreenY;
    wxPoint mHandlePosition[5];
    int mDragMode;
    int mLastResizeX;
    wxXmlNode* ModelXml;
    
    std::vector<std::string> strandNames;
    std::vector<std::string> nodeNames;
    long parm1;         /* Number of strings in the model or number of arches (except for frames & custom) */
    long parm2;         /* Number of nodes per string in the model or number of segments per arch (except for frames & custom) */
    long parm3;         /* Number of strands per string in the model or number of lights per arch segment (except for frames & custom) */
    bool IsLtoR;         // true=left to right, false=right to left
    std::vector<long> stringStartChan;
    bool isBotToTop;
    std::string StringType; // RGB Nodes, 3 Channel RGB, Single Color Red, Single Color Green, Single Color Blue, Single Color White
    std::string DisplayAs;  // Tree 360, Tree 270, Tree 180, Tree 90, Vert Matrix, Horiz Matrix, Single Line, Arches, Window Frame
    
public:
    std::string rgbOrder;
    bool SingleNode;     // true for dumb strings and single channel strings
    bool SingleChannel;  // true for traditional single-color strings
    
    int RenderHt,RenderWi;  // size of the rendered output
    bool MyDisplay;
    bool Selected=false;
    bool GroupSelected=false;
    std::string ModelStartChannel;
    const NetInfoClass * ModelNetInfo;
    bool Overlapping=false;
    void SetFromXml(wxXmlNode* ModelNode, const NetInfoClass &netInfo, bool zeroBased=false);
    size_t GetNodeCount() const;
    int GetChanCount() const;
    int GetChanCountPerNode() const;
    size_t GetCoordCount(size_t nodenum);
    void UpdateXmlWithScale();
    void SetOffset(double xPct, double yPct);
    void AddOffset(double xPct, double yPct);
    void SetScale(double x, double y);
    void GetScales(double &x, double &y);
    int GetPreviewRotation();
    int GetLastChannel();
    int GetNodeNumber(size_t nodenum);
    wxXmlNode* GetModelXml() const;
    int GetNumberFromChannelString(std::string sc);
    wxCursor GetResizeCursor(int cornerIndex);
    void DisplayModelOnWindow(ModelPreview* preview, const xlColor *color =  NULL, bool allowSelected = true);
    void DisplayEffectOnWindow(ModelPreview* preview, double pointSize);
    void ResizeWithHandles(ModelPreview* preview, int mouseX,int mouseY);
    void RotateWithHandles(ModelPreview* preview,bool ShiftKeyPressed,  int mouseX,int mouseY);
    bool HitTest(ModelPreview* preview,int x,int y);
    bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2);
    void AddToWholeHouseModel(int pw, int ph, std::vector<int>& xPos,std::vector<int>& yPos,
                              std::vector<int>& actChannel,std::vector<std::string>& nodeTypes);
    void SetMinMaxModelScreenCoordinates(ModelPreview* preview);
    void Rotate(int degrees);
    const std::string& GetStringType(void) const { return StringType; }
    const std::string& GetDisplayAs(void) const { return DisplayAs; }
    int NodesPerString();
    void SetModelCoord(int degrees);
    int CheckIfOverHandles(ModelPreview* preview, wxCoord x,wxCoord y);
    int GetRotation();
    int NodeStartChannel(size_t nodenum) const;
    const std::string &NodeType(size_t nodenum) const;
    virtual int MapToNodeIndex(int strand, int node) const;
    void SetModelStartChan(const std::string &start_channel);
    int ChannelStringToNumber(std::string channel);
    
    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char *buf);
    xlColor GetNodeColor(size_t nodenum) const;
    wxChar GetChannelColorLetter(wxByte chidx);
    
    std::string ChannelLayoutHtml();
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
    std::string GetStrandName(int x, bool def = false) const {
        if (x < strandNames.size()) {
            return strandNames[x];
        }
        if (def) {
            return wxString::Format("Strand %d", x + 1).ToStdString();
        }
        return "";
    }
    std::string GetNodeName(int x, bool def = false) const {
        if (x < nodeNames.size()) {
            return nodeNames[x];
        }
        if (def) {
            return wxString::Format("Node %d", x + 1).ToStdString();
        }
        return "";
    }
    static bool IsMyDisplay(wxXmlNode* ModelNode);
    static void SetMyDisplay(wxXmlNode* ModelNode,bool NewValue);
    
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
    
};

#endif // MODEL_H
