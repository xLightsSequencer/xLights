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
#include <wx/xml/xml.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/window.h>
#include <wx/dcclient.h>
#include <stdint.h>
#include <wx/choice.h>

typedef std::vector<long> StartChannelVector_t;

#define NODE_RGB_CHAN_CNT 3
#define NODE_SINGLE_COLOR_CHAN_CNT 1

class ModelClass
{
private:

    class NodeBaseClass
    {
    private:

        // buffer and screen coordinates for displayed nodes
        struct CoordStruct
        {
            wxCoord bufX, bufY, screenX, screenY;
        };

    protected:
        // color values in rgb order
        uint8_t c[3];
        int chanCnt;

    public:

        int sparkle;
        int ActChan;   // 0 is the first channel
        int StringNum; // node is part of this string (0 is the first string)
        std::vector<CoordStruct> Coords;

        // only for use in initializing the custom model
        void AddBufCoord(wxCoord x, wxCoord y)
        {
            CoordStruct c;
            c.bufX=x;
            c.bufY=y;
            Coords.push_back(c);
        }

        void SetColor(wxColour& color)
        {
            c[0]=color.Red();
            c[1]=color.Green();
            c[2]=color.Blue();
        }

        void SetColor(uint8_t r,uint8_t g,uint8_t b)
        {
            c[0]=r;
            c[1]=g;
            c[2]=b;
        }

        void GetChanIntensity(wxByte chnum, wxByte rgbindex, size_t *absChNum, uint8_t *intensity)
        {
            *absChNum=ActChan+chnum;
            *intensity=c[rgbindex];
        }

        void SetChanIntensity(wxByte rgbindex, uint8_t intensity)
        {
            c[rgbindex]=intensity;
        }

        void SetChanIntensityAll(uint8_t intensity)
        {
            c[0]=intensity;
            c[1]=intensity;
            c[2]=intensity;
        }

        int GetChanCount()
        {
            return chanCnt;
        }
        bool IsVisible()
        {
            return Coords.size() > 0;
        }

        NodeBaseClass()
        {
            chanCnt=NODE_RGB_CHAN_CNT;
        }

        NodeBaseClass(int StringNumber, size_t NodesPerString)
        {
            StringNum=StringNumber;
            Coords.resize(NodesPerString);
            chanCnt=NODE_RGB_CHAN_CNT;
        }

        virtual ~NodeBaseClass()
        {
        }

        virtual void GetColor(wxColour& color)
        {
            color.Set(c[0],c[1],c[2]);
        }
    };

    class NodeClassRed : public NodeBaseClass
    {
    public:
        NodeClassRed(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        }
        virtual void GetColor(wxColour& color)
        {
            color.Set(c[0],0,0);
        }
    };

    class NodeClassGreen : public NodeBaseClass
    {
    public:
        NodeClassGreen(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        }
        virtual void GetColor(wxColour& color)
        {
            color.Set(0,c[1],0);
        }
    };

    class NodeClassBlue : public NodeBaseClass
    {
    public:
        NodeClassBlue(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        }
        virtual void GetColor(wxColour& color)
        {
            color.Set(0,0,c[2]);
        }
    };

    class NodeClassWhite : public NodeBaseClass
    {
    public:
        NodeClassWhite(int StringNumber, size_t NodesPerString) : NodeBaseClass(StringNumber,NodesPerString)
        {
            chanCnt = NODE_SINGLE_COLOR_CHAN_CNT;
        }

        virtual void GetColor(wxColour& color)
        {
            uint8_t cmin=std::min(c[0],std::min(c[1],c[2]));
            color.Set(cmin,cmin,cmin);
        }
    };

    typedef std::unique_ptr<NodeBaseClass> NodeBaseClassPtr;

    void InitVMatrix();
    void InitHMatrix();
    void InitLine();
    void InitFrame();
    void InitStar();
    void InitWreath();

    void SetBufferSize(int NewHt, int NewWi);
    void SetRenderSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NumStrings, size_t NodesPerString);
    void CopyBufCoord2ScreenCoord();
    void SetTreeCoord(long degrees);
    void SetLineCoord();
    void SetArchCoord();
    int GetCustomMaxChannel(const wxString& customModel);
    void InitCustomMatrix(const wxString& customModel);
    double toRadians(long degrees);
    int NodesPerString();

    wxString DisplayAs;  // Tree 360, Tree 270, Tree 180, Tree 90, Vert Matrix, Horiz Matrix, Single Line, Arches, Window Frame
    wxString StringType; // RGB Nodes, 3 Channel RGB, Single Color Red, Single Color Green, Single Color Blue, Single Color White
    long parm1;         /**< Number of strings in the model (except for frames & custom) */
    long parm2;         /**< Number of nodes per string in the model (except for frames & custom) */
    long parm3;         /**< Number of strands per string in the model (except for frames & custom) */
    bool IsLtoR;         // true=left to right, false=right to left
    bool isBotToTop;
    long Antialias;      // amount of anti-alias (0,1,2)
    int AliasFactor;     // factor to expand buffer (2 ^ Antialias)

    int TreeDegrees,FrameWidth;
    bool modelv2;
    int StrobeRate;      // 0=no strobing
    double offsetXpct,offsetYpct;
    double PreviewScale;
    int PreviewRotation;
    bool SingleNode;     // true for dumb strings and single channel strings
    bool SingleChannel;  // true for traditional single-color strings

    StartChannelVector_t stringStartChan;
    wxXmlNode* ModelXml;
    wxByte rgbidx[3]; // records the order in which the color values are sent to the physical device
    // rgbidx entries should be 0-2
    // for rgb order, set: 0,1,2
    // for grb order, set: 1,0,2
    // for brg order, set: 2,0,1

protected:
    std::vector<NodeBaseClassPtr> Nodes;

public:
    wxString name;       // user-designated model name
    int BufferHt,BufferWi;  // size of the buffer
    int RenderHt,RenderWi;  // size of the rendered output
    bool MyDisplay;
    long ModelBrightness;   // Value from -100 to +100 indicates an adjustment to brightness for this model

    void SetFromXml(wxXmlNode* ModelNode, bool zeroBased=false);
    size_t GetNodeCount();
    int GetChanCount();
    size_t GetCoordCount(size_t nodenum);
    void UpdateXmlWithScale();
    void SetOffset(double xPct, double yPct);
    void AddOffset(double xPct, double yPct);
    void SetScale(double newscale);
    double GetScale();
    int GetLastChannel();
    int GetNodeNumber(size_t nodenum);
    void DisplayModelOnWindow(wxWindow* window);
    void DisplayModelOnWindow(wxWindow* window, const wxColour* color);
    void DisplayEffectOnWindow(wxWindow* window);
    bool CanRotate();
    void Rotate(int degrees);

    int GetRotation();
    int ChannelsPerNode();
    int NodeStartChannel(size_t nodenum);
    void GetChanIntensity(size_t nodenum,wxByte chidx, size_t *absChNum, uint8_t *intensity);
    void SetChanIntensity(size_t nodenum,wxByte chidx, uint8_t intensity);
    void SetChanIntensityAll(size_t nodenum, uint8_t intensity);
    wxString ChannelLayoutHtml();
    wxChar GetChannelColorLetter(wxByte chidx);
//    int FindChannelAt(int x, int y);
//    wxSize GetChannelCoords(std::vector<std::vector<int>>& chxy, bool shrink); //for pgo RenderFaces
    size_t GetChannelCoords(wxChoice* choices);
//    int FindChannelAtXY(int x, int y, const wxString& model);

    long GetNumArches()
    {
        if (DisplayAs == wxT("Arches"))
            return parm1;
        else
            return 0;
    }

    long GetNodesPerArch()
    {
        if (DisplayAs == wxT("Arches"))
            return parm2;
        else
            return 0;
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
        return StrType.StartsWith(wxT("Single")) || StrType.StartsWith(wxT("Strobe"));
    }

};
typedef std::unique_ptr<ModelClass> ModelClassPtr;

#endif // MODELCLASS_H
