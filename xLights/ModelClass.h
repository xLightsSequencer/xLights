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

#include <vector>
#include <wx/xml/xml.h>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/window.h>
#include <wx/dcclient.h>
#include <stdint.h>

class RgbNodeClass
{
    uint8_t c[3];      // color values in rgb order
    uint8_t offset[3]; // offsets to change the order in which the color values are sent to the physical device

public:

    wxCoord bufX, bufY, screenX, screenY;
    size_t bufIdx;
    int sparkle;
    int ActChan;   // 0 is the first channel
    int StringNum; // node is part of this string # - used only for reporting purposes. 0 is the first string.

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

    // offsets should be 0-2
    // for rgb order, pass: 0,1,2
    // for grb order, pass: 1,0,2
    // for brg order, pass: 2,0,1
    void SetOffset(uint8_t offset_r,uint8_t offset_g,uint8_t offset_b)
    {
        offset[0]=offset_r;
        offset[1]=offset_g;
        offset[2]=offset_b;
    }

    // chnum should be 0-2
    uint8_t GetChannelColorVal(size_t chnum)
    {
        return c[offset[chnum]];
    }

    int getChanNum(size_t chnum)
    {
        return ActChan+chnum;
    }

    void getRGBChanNum(size_t *rChNum, size_t *gChNum, size_t *bChNum)
    {
        *rChNum=ActChan+offset[0];
        *gChNum=ActChan+offset[1];
        *bChNum=ActChan+offset[2];
    }

    void GetColor(wxColour& color)
    {
        color.Set(c[0],c[1],c[2]);
    }
};

uint8_t * base64_decode(const wxString& encoded_string, int rows, int cols);
wxString base64_encode(uint8_t *chanData, int length);

typedef std::vector<RgbNodeClass> RgbNodeVector;
typedef std::vector<long> StartChannelVector_t;

class ModelClass
{
private:
    void InitVMatrix();
    void InitHMatrix();
    void InitLine();
    void InitFrame();
    void SetBufferSize(int NewHt, int NewWi);
    void SetRenderSize(int NewHt, int NewWi);
    void SetNodeCount(size_t NewCount);
    void CopyBufCoord2ScreenCoord();
    void SetTreeCoord(long degrees);
    void SetLineCoord();
    void SetArchCoord();
    void InitCustomMatrix(wxString customModel);
    void SetFromXmlAdvanced(wxXmlNode* ModelNode);
    void InitializeStringStartNum();
    double toRadians(long degrees);

    bool modelv2;
    double offsetXpct,offsetYpct;
    double PreviewScale;
    int PreviewRotation;
    wxXmlNode* ModelXml;

public:
    wxString name;      // user-designated model name
    wxString DisplayAs; // Tree 360, Tree 270, Tree 180, Tree 90, Vert Matrix, Horiz Matrix, Single Line, Arches, Window Frame
    wxString RGBorder;  // RGB, RBG, GBR, GRB, BGR, BRG
    bool IsLtoR;        // true=left to right, false=right to left
    bool isBotToTop;
    long parm1;         /**< Number of strings in the model (except for frames & custom) */
    long parm2;         /**< Number of nodes per string in the model (except for frames & custom) */
    long parm3;         /**< Number of strands per string in the model (except for frames & custom) */
    long StartChannel;      // start channel for output (1 is first channel)
    int BufferHt,BufferWi;  // size of the buffer
    int RenderHt,RenderWi;  // size of the rendered output
    long Antialias;         // amount of anti-alias (0,1,2)
    int AliasFactor;        // factor to expand buffer (2 ^ Antialias)
    int TreeDegrees,FrameWidth;
    RgbNodeVector Nodes;
    bool MyDisplay;

    StartChannelVector_t stringStartChan;

    void SetFromXml(wxXmlNode* ModelNode);
    size_t GetNodeCount();
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

    static bool IsMyDisplay(wxXmlNode* ModelNode) {
        return ModelNode->GetAttribute(wxT("MyDisplay"),wxT("0")) == wxT("1");
    }
    static void SetMyDisplay(wxXmlNode* ModelNode,bool NewValue) {
        ModelNode->DeleteAttribute(wxT("MyDisplay"));
        ModelNode->AddAttribute(wxT("MyDisplay"), NewValue ? wxT("1") : wxT("0"));
    }
};

#endif // MODELCLASS_H
