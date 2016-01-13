/***************************************************************
 * Name:      ModelClass.h
 * Purpose:   Represents Display Model
 * Author:    Matt Brown (dowdybrown@yahoo.com)
 * Created:   2012-10-22
 * Copyright: 2012 by Matt Brown
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#ifndef MODELCLASS_H
#define MODELCLASS_H

#include <memory>
#include <vector>
#include <map>
#include <wx/xml/xml.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/window.h>
#include <wx/dcclient.h>
#include <stdint.h>
#include <wx/choice.h>
#include <wx/checklst.h>
#include <wx/listbox.h>
#include <wx/tokenzr.h>


#include "Color.h"

typedef std::vector<long> StartChannelVector_t;
class NetInfoClass;
class ModelPreview;
class DimmingCurve;

#define RECT_HANDLE_WIDTH           6
#define BOUNDING_RECT_OFFSET        8

#include "models/Node.h"

class ModelClass
{
private:

    static NodeBaseClass* createNode(int ns, const std::string &StringType, size_t NodesPerString, const std::string &rgbOrder);

protected:
    void InitLine();
private:
    void InitVMatrix(int firstExportStrand = 0);
    void InitHMatrix();
    void InitArches();
    void InitCircle();
    void InitFrame();
    void InitStar();
    void InitWreath();
    void InitSphere();
    void InitWholeHouse(const wxString &data, bool zeroBased = false);

    void SetBufferSize(int NewHt, int NewWi);
    void SetRenderSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NumStrings, size_t NodesPerString, const std::string &rgbOrder);
    void CopyBufCoord2ScreenCoord();
    void SetTreeCoord(long degrees);

    void SetLineCoord();
    void SetArchCoord();
    void SetCircleCoord();
    int GetCustomMaxChannel(const wxString& customModel);
    void InitCustomMatrix(const wxString& customModel);
    double toRadians(long degrees);
    long toDegrees(double radians);
    std::string GetNextName();


    std::vector<int> starSizes;
    std::vector<int> circleSizes;

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

    int mMinScreenX;
    int mMinScreenY;
    int mMaxScreenX;
    int mMaxScreenY;
    wxPoint mHandlePosition[5];
    int mDragMode;
    int mLastResizeX;
    wxXmlNode* ModelXml;
protected:

    xlColor customColor;
    std::vector<NodeBaseClassPtr> Nodes;
    std::vector<wxString> strandNames;
    std::vector<wxString> nodeNames;
    wxString rgbOrder;
    long parm1;         /* Number of strings in the model or number of arches (except for frames & custom) */
    long parm2;         /* Number of nodes per string in the model or number of segments per arch (except for frames & custom) */
    long parm3;         /* Number of strands per string in the model or number of lights per arch segment (except for frames & custom) */
    bool IsLtoR;         // true=left to right, false=right to left
    bool SingleNode;     // true for dumb strings and single channel strings
    bool SingleChannel;  // true for traditional single-color strings
    StartChannelVector_t stringStartChan;
    bool isBotToTop;
    wxString StringType; // RGB Nodes, 3 Channel RGB, Single Color Red, Single Color Green, Single Color Blue, Single Color White
    wxString DisplayAs;  // Tree 360, Tree 270, Tree 180, Tree 90, Vert Matrix, Horiz Matrix, Single Line, Arches, Window Frame

    friend class PixelBufferClass;
public:
    ModelClass();
    virtual ~ModelClass();

    wxString name;       // user-designated model name
    int BufferHt,BufferWi;  // size of the buffer
    int RenderHt,RenderWi;  // size of the rendered output
    std::map<wxString, std::map<wxString, wxString> > faceInfo;
    DimmingCurve *modelDimmingCurve;
    bool MyDisplay;
    bool Selected=false;
    bool GroupSelected=false;
    wxString ModelStartChannel;
    NetInfoClass *ModelNetInfo;
    bool Overlapping=false;
    void SetFromXml(wxXmlNode* ModelNode, NetInfoClass &netInfo, bool zeroBased=false);
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
    wxXmlNode* GetModelXml();
    int GetNumberFromChannelString(wxString sc);
    wxCursor GetResizeCursor(int cornerIndex);
    void DisplayModelOnWindow(ModelPreview* preview, const xlColour *color =  NULL, bool allowSelected = true);
    void DisplayEffectOnWindow(ModelPreview* preview, double pointSize);
    void ResizeWithHandles(ModelPreview* preview, int mouseX,int mouseY);
    void RotateWithHandles(ModelPreview* preview,bool ShiftKeyPressed,  int mouseX,int mouseY);
    bool HitTest(ModelPreview* preview,int x,int y);
    bool IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2);
    void AddToWholeHouseModel(ModelPreview* preview,std::vector<int>& xPos,std::vector<int>& yPos,
                              std::vector<int>& actChannel,std::vector<wxString>& nodeTypes);
    void SetMinMaxModelScreenCoordinates(ModelPreview* preview);
    bool CanRotate();
    void Rotate(int degrees);
    const wxString& GetStringType(void) const { return StringType; }
    const wxString& GetDisplayAs(void) const { return DisplayAs; }
    int NodesPerString();
    void SetModelCoord(int degrees);
    int CheckIfOverHandles(ModelPreview* preview, wxCoord x,wxCoord y);
    int GetRotation();
    int ChannelsPerNode();
    int NodeStartChannel(size_t nodenum) const;
    wxString NodeType(size_t nodenum) const;
    int MapToNodeIndex(int strand, int node) const;
	void SetModelStartChan(wxString start_channel);
    int ChannelStringToNumber(wxString channel);

    void GetNodeChannelValues(size_t nodenum, unsigned char *buf);
    void SetNodeChannelValues(size_t nodenum, const unsigned char *buf);
    xlColor GetNodeColor(size_t nodenum);
    wxChar GetChannelColorLetter(wxByte chidx);

    wxString ChannelLayoutHtml();
//    int FindChannelAt(int x, int y);
//    wxSize GetChannelCoords(std::vector<std::vector<int>>& chxy, bool shrink); //for pgo RenderFaces
    bool IsCustom(void);
    size_t GetChannelCoords(wxArrayString& choices); //wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3);
    static bool ParseFaceElement(const wxString& str, std::vector<wxPoint>& first_xy);
//    int FindChannelAtXY(int x, int y, const wxString& model);
    wxString GetNodeXY(const wxString& nodenumstr);
    wxString GetNodeXY(int nodeinx);

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

    bool GetIsLtoR() {return IsLtoR;}
    bool GetIsBtoT() {return isBotToTop;}

    int GetStrandLength(int strand) const;

    long GetNumArches() const
    {
        if (DisplayAs == wxT("Arches"))
            return parm1;
        else
            return 0;
    }

    long GetNodesPerArch() const
    {
        if (DisplayAs == wxT("Arches"))
            return parm2;
        else
            return 0;
    }

    int GetNumStrands() const {
        wxStringTokenizer tkz(DisplayAs, " ");
        wxString token = tkz.GetNextToken();
        if (DisplayAs == wxT("Arches"))
            return parm1;
        else if (token == wxT("Tree"))
            return parm1*parm3;
        else if (DisplayAs == wxT("Vert Matrix") || DisplayAs == wxT("Horiz Matrix")) {
            if (SingleChannel) {
                return 1;
            }
            return parm1*parm3;
        } else if (token == wxT("Star"))
            return starSizes.size();
        else if (token == wxT("Circle"))
            return circleSizes.size();
        else
            return 1;
    }
    int GetStarSize(int starLayer) const {
        return starSizes[starLayer];
    }
    int GetCircleSize(int circleLayer) const {
        return circleSizes[circleLayer];
    }
    wxString GetStrandName(int x, bool def = false) const {
        if (x < strandNames.size()) {
            return strandNames[x];
        }
        if (def) {
            return wxString::Format("Strand %d", x + 1);
        }
        return "";
    }
    wxString GetNodeName(int x, bool def = false) const {
        if (x < nodeNames.size()) {
            return nodeNames[x];
        }
        if (def) {
            return wxString::Format("Node %d", x + 1);
        }
        return "";
    }
    static bool IsMyDisplay(wxXmlNode* ModelNode)
    {
        return ModelNode->GetAttribute(wxT("MyDisplay"),wxT("0")) == wxT("1");
    }
    static void SetMyDisplay(wxXmlNode* ModelNode,bool NewValue)
    {
        ModelNode->DeleteAttribute(wxT("MyDisplay"));
        ModelNode->AddAttribute(wxT("MyDisplay"), NewValue ? wxT("1") : wxT("0"));
    }
    static wxString StartChanAttrName(int idx)
    {
        return wxString::Format(wxT("String%d"),idx+1);
    }
    // returns true for models that only have 1 string and where parm1 does NOT represent the # of strings
    static bool HasOneString(const wxString& DispAs)
    {
        return (DispAs == wxT("Window Frame") || DispAs == wxT("Custom"));
    }
    // true for dumb strings and traditional strings
    static bool HasSingleNode(const wxString& StrType)
    {
        return !StrType.EndsWith(wxT(" Nodes"));
    }
    // true for traditional strings
    static bool HasSingleChannel(const wxString& StrType)
    {
        return GetNodeChannelCount(StrType) == 1;
    }
    static int GetNodeChannelCount(const wxString & nodeType);

};
typedef std::unique_ptr<ModelClass> ModelClassPtr;

#endif // MODELCLASS_H
