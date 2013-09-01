/***************************************************************
 * Name:      ModelClass.cpp
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

#include "ModelClass.h"
#include <wx/msgdlg.h>
#include <wx/tokenzr.h>

void ModelClass::SetFromXml(wxXmlNode* ModelNode)
{
    wxString tempstr;
    wxString customModel;
    long degrees;

    ModelXml=ModelNode;
    name=ModelNode->GetAttribute(wxT("name"));
    DisplayAs=ModelNode->GetAttribute(wxT("DisplayAs"));
    RGBorder=ModelNode->GetAttribute(wxT("Order"),wxT("RGB"));
    tempstr=ModelNode->GetAttribute(wxT("parm1"));
    tempstr.ToLong(&parm1);
    tempstr=ModelNode->GetAttribute(wxT("parm2"));
    tempstr.ToLong(&parm2);
    tempstr=ModelNode->GetAttribute(wxT("parm3"));
    tempstr.ToLong(&parm3);
    tempstr=ModelNode->GetAttribute(wxT("StartChannel"),wxT("1"));
    tempstr.ToLong(&StartChannel);
    tempstr=ModelNode->GetAttribute(wxT("Dir"));
    IsLtoR=tempstr != wxT("R");
    if (ModelNode->HasAttribute(wxT("StartSide")))
    {
        tempstr=ModelNode->GetAttribute(wxT("StartSide"));
        isBotToTop = (tempstr == wxT("B"));
    }
    else
    {
        isBotToTop=true;
    }
    if (ModelNode->HasAttribute(wxT("CustomModel")))
    {
        customModel = ModelNode->GetAttribute(wxT("CustomModel"));
    }

    tempstr=ModelNode->GetAttribute(wxT("Antialias"),wxT("0"));
    tempstr.ToLong(&Antialias);
    AliasFactor=1 << Antialias;
    MyDisplay=IsMyDisplay(ModelNode);

    tempstr=ModelNode->GetAttribute(wxT("offsetXpct"),wxT("0"));
    tempstr.ToDouble(&offsetXpct);
    tempstr=ModelNode->GetAttribute(wxT("offsetYpct"),wxT("0"));
    tempstr.ToDouble(&offsetYpct);
    tempstr=ModelNode->GetAttribute(wxT("PreviewScale"),wxT("0.333"));
    tempstr.ToDouble(&PreviewScale);

    wxStringTokenizer tkz(DisplayAs, wxT(" "));
    wxString token = tkz.GetNextToken();

    modelv2 = ModelNode->HasAttribute(wxT("Advanced"));
    if( modelv2 )
    {
        SetFromXmlAdvanced(ModelNode);
    }
    else
    {
        InitializeStringStartNum();
    }


    if (token == wxT("Tree"))
    {
        InitVMatrix();
        token = tkz.GetNextToken();
        token.ToLong(&degrees);
        SetTreeCoord(degrees);
    }
    else if (DisplayAs == wxT("Custom"))
    {
        InitCustomMatrix(customModel);
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == wxT("Vert Matrix"))
    {
        InitVMatrix();
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == wxT("Horiz Matrix"))
    {
        InitHMatrix();
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == wxT("Single Line"))
    {
        InitLine();
        SetLineCoord();
    }
    else if (DisplayAs == wxT("Arches"))
    {
        InitLine();
        SetArchCoord();
    }
    else if (DisplayAs == wxT("Window Frame"))
    {
        InitFrame();
        CopyBufCoord2ScreenCoord();
    }
    size_t NodeCount=GetNodeCount();
    uint8_t offset_r=RGBorder.find(wxT("R"));
    uint8_t offset_g=RGBorder.find(wxT("G"));
    uint8_t offset_b=RGBorder.find(wxT("B"));
    for(size_t i=0; i<NodeCount; i++)
    {
        Nodes[i].SetOffset(offset_r, offset_g, offset_b);
        Nodes[i].sparkle = rand() % 10000;
    }
}

int ModelClass::GetLastChannel()
{
    int LastChan=0;
    size_t NodeCount=GetNodeCount();
    for(size_t idx=0; idx<NodeCount; idx++)
    {
        LastChan=std::max(LastChan,Nodes[idx].ActChan+2);
    }
    return LastChan;
}

void ModelClass::SetOffset(double xPct, double yPct)
{
    offsetXpct=xPct;
    offsetYpct=yPct;
}


void ModelClass::AddOffset(double xPct, double yPct)
{
    offsetXpct+=xPct;
    offsetYpct+=yPct;
}


void ModelClass::SetScale(double newscale)
{
    PreviewScale=newscale;
}

double ModelClass::GetScale()
{
    return PreviewScale;
}

void ModelClass::InitializeStringStartNum()
{
    int i;
    stringStartChan.clear();
    stringStartChan.resize(parm1);
    for (i=0; i<parm1; i++)
    {
        stringStartChan[i] = StartChannel-1 + (i)*parm2*3;
    }
}
void ModelClass::SetFromXmlAdvanced(wxXmlNode* ModelNode)
{
    wxString strText;
    wxString idxString;
    wxString tmpStr;
    int stringNum = 0;
    long val;

    strText = wxT("String");
    stringStartChan.clear();
    stringStartChan.resize(parm1);

    for(idxString = strText.Left(6).Append((wxString::Format(wxT("%i"),stringNum+1)));
        true == ModelNode->GetAttribute(idxString, &tmpStr) ;
        stringNum++, idxString = strText.Left(6).Append((wxString::Format(wxT("%i"),stringNum+1))))
    {
        tmpStr.ToLong(&val);
        stringStartChan[stringNum] = val-1;
    }
    if (stringNum != parm1)
    {
        //ERROR: not equal number of strings and start channels
    }

}
// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void ModelClass::InitVMatrix()
{
    int y,x,idx,stringnum,segmentnum;
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    SetBufferSize(PixelsPerStrand,NumStrands);
    SetNodeCount(parm1*parm2);
    SetRenderSize(PixelsPerStrand,NumStrands);

    // create output mapping
    for (x=0; x < NumStrands; x++)
    {
        stringnum=x / parm3;
        segmentnum=x % parm3;
        for(y=0; y < PixelsPerStrand; y++)
        {
            idx=stringnum * parm2 + segmentnum * PixelsPerStrand + y;
            Nodes[idx].ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + y*3;
            Nodes[idx].bufX=IsLtoR ? x : NumStrands-x-1;
            Nodes[idx].bufY= isBotToTop == (segmentnum % 2 == 0) ? y:PixelsPerStrand-y-1;
            Nodes[idx].StringNum=stringnum;
        }
    }
}
void ModelClass::InitCustomMatrix(wxString customModel)
{
    wxString value;
    wxArrayString cols;
    long idx;
    int width=1;
    RgbNodeClass node;

    Nodes.clear();
    wxArrayString rows=wxSplit(customModel,';');
    int height=rows.size();
    node.StringNum=0;
    for(size_t row=0; row < rows.size(); row++)
    {
        cols=wxSplit(rows[row],',');
        if (cols.size() > width) width=cols.size();
        for(size_t col=0; col < cols.size(); col++)
        {
            value=cols[col];
            if (!value.IsEmpty() && value != wxT("0"))
            {
                // add a node
                value.ToLong(&idx);
                node.ActChan = (StartChannel-1) + ((idx-1) * 3);
                node.bufX=col;
                node.bufY=height - row - 1;
                Nodes.push_back(node);
            }
        }
    }
    SetBufferSize(height,width);
    SetRenderSize(height,width);
}

// initialize screen coordinates for tree
void ModelClass::SetTreeCoord(long degrees)
{
    int bufferX, bufferY;
    double angle,x0;
    TreeDegrees=degrees;
    if (BufferWi < 2) return;
    if(BufferHt<1) return; // June 27,2013. added check to not divide by zero
    int factor=1000/BufferHt;
    RenderHt=BufferHt*factor;
    RenderWi=RenderHt/2;
    double radians=2.0*M_PI*double(degrees)/360.0;
    double radius=RenderWi/2.0;
    double StartAngle=-radians/2.0;
    double AngleIncr=radians/double(BufferWi-1);
    //wxString msg=wxString::Format(wxT("BufferHt=%d, BufferWi=%d, factor=%d, RenderHt=%d, RenderWi=%d\n"),BufferHt,BufferWi,factor,RenderHt,RenderWi);
    size_t NodeCount=GetNodeCount();
    for(size_t idx=0; idx<NodeCount; idx++)
    {
        bufferX=Nodes[idx].bufX;
        bufferY=Nodes[idx].bufY;
        angle=StartAngle + double(bufferX) * AngleIncr;
        x0=radius * sin(angle);
        Nodes[idx].screenX=floor(x0*(1.0-double(bufferY)/double(BufferHt)) + 0.5);
        Nodes[idx].screenY=bufferY * factor;
        //if (bufferY==0) msg+=wxString::Format(wxT("idx=%d, bufX=%d, screenX=%d\n"),idx,bufferX,Nodes[idx].screenX);
    }
    //wxMessageBox(msg);
}

// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void ModelClass::InitHMatrix()
{
    int y,x,idx,stringnum,segmentnum;
    TreeDegrees=0;
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    SetBufferSize(NumStrands,PixelsPerStrand);
    SetNodeCount(parm1*parm2);
    SetRenderSize(NumStrands,PixelsPerStrand);

    // create output mapping
    for (y=0; y < NumStrands; y++)
    {
        stringnum=y / parm3;
        segmentnum=y % parm3;
        for(x=0; x<PixelsPerStrand; x++)
        {
            idx=stringnum * parm2 + segmentnum * PixelsPerStrand + x;
            Nodes[idx].ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + x*3;
            Nodes[idx].bufX=IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand-x-1 : x;
            Nodes[idx].bufY= isBotToTop ? y :NumStrands-y-1;
            Nodes[idx].StringNum=stringnum;
        }
    }
}

// initialize buffer and screen coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void ModelClass::InitLine()
{
    int ns,x;
    int idx=0;
    int NodeCount=parm1*parm2;
    TreeDegrees=0;
    SetBufferSize(1,parm2);
    SetNodeCount(NodeCount);
    for (ns=0; ns < parm1; ns++)
    {
        for(x=0; x<parm2; x++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + idx * 3;
            Nodes[idx].bufX=IsLtoR ? x : parm2-x-1;
            Nodes[idx].bufY=0;
            Nodes[idx].StringNum=ns;
            idx++;
        }
    }
}

void ModelClass::SetLineCoord()
{
    size_t NodeCount=GetNodeCount();
    int idx=0;
    SetRenderSize(1,NodeCount);
    int xoffset=RenderWi/2;
    for(size_t x=0; x<NodeCount; x++)
    {
        Nodes[idx].screenX=(IsLtoR ? idx : NodeCount-idx-1) - xoffset;
        Nodes[idx].screenY=0;
        idx++;
    }
}

// Set screen coordinates for arches
void ModelClass::SetArchCoord()
{
    int ns,x, xoffset, idx, incr;
    double angle;
    size_t NodeCount=GetNodeCount();
    if (IsLtoR)
    {
        idx=0;
        incr=1;
    }
    else
    {
        idx=NodeCount-1;
        incr=-1;
    }
    SetRenderSize(parm2,NodeCount*2);
    double midpt=parm2;
    double AngleIncr=M_PI / parm2;
    for (ns=0; ns < parm1; ns++)
    {
        angle=-1.0*M_PI/2.0;
        xoffset=ns*parm2*2 - NodeCount;
        for(x=0; x<parm2; x++)
        {
            Nodes[idx].screenX=xoffset + (int)floor(midpt*sin(angle)+midpt);
            Nodes[idx].screenY=(int)floor(midpt*cos(angle)+0.5);
            angle+=AngleIncr;
            idx+=incr;
        }
    }
}

// initialize buffer coordinates
// parm1=Nodes on Top
// parm2=Nodes left and right
// parm3=Nodes on Bottom
void ModelClass::InitFrame()
{
    int x,y;
    int idx=0;
    TreeDegrees=0;
    SetNodeCount(parm1+2*parm2+parm3);
    if (parm1 >= parm3)
    {
        // first node is bottom left and we count up the left side, across the top, and down the right
        FrameWidth=parm1+2;  // allow for left/right columns

        // up side 1
        x=IsLtoR ? 0 : FrameWidth-1;
        for(y=0; y<parm2; y++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across top
        y=parm2-1;
        for(x=0; x<parm1; x++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=IsLtoR ? x+1 : parm1-x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // down side 2
        x=IsLtoR ? FrameWidth-1 : 0;
        for(y=parm2-1; y>=0; y--)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across bottom
        y=0;
        for(x=0; x<parm3; x++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=IsLtoR ? parm1-x : x+1;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
    }
    else
    {
        // first node is top left and we count down the left side, across the bottom, and up the right
        FrameWidth=parm3+2;

        // down side 1
        x=IsLtoR ? 0 : FrameWidth-1;
        for(y=parm2-1; y>=0; y--)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across bottom
        y=0;
        for(x=0; x<parm3; x++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=IsLtoR ? x+1: parm3-x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // up side 2
        x=IsLtoR ? FrameWidth-1 : 0;
        for(y=0; y<parm2; y++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across top
        y=parm2-1;
        for(x=0; x<parm1; x++)
        {
            Nodes[idx].ActChan = (StartChannel-1) + (idx * 3);
            Nodes[idx].bufX=IsLtoR ? parm3-x : x+1;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
    }
    // treat as outside of matrix
    SetBufferSize(parm2,FrameWidth);

    // treat as single string
    //SetBufferSize(1,Nodes.size());
    SetRenderSize(parm2,FrameWidth);
}

void ModelClass::SetBufferSize(int NewHt, int NewWi)
{
    BufferHt=NewHt;
    BufferWi=NewWi;
}

void ModelClass::SetRenderSize(int NewHt, int NewWi)
{
    RenderHt=NewHt;
    RenderWi=NewWi;
}

void ModelClass::SetNodeCount(size_t NewCount)
{
    Nodes.clear();
    Nodes.resize(NewCount);
    for(size_t i=0; i<Nodes.size(); i++)
    {
        Nodes[i].bufX = -1;  // set to invalid
    }
}

size_t ModelClass::GetNodeCount()
{
    return Nodes.size();
}

// initialize screen coordinates
void ModelClass::CopyBufCoord2ScreenCoord()
{
    size_t NodeCount=GetNodeCount();
    int xoffset=RenderWi/2;
    for(size_t i=0; i<NodeCount; i++)
    {
        Nodes[i].screenX = Nodes[i].bufX - xoffset;
        Nodes[i].screenY = Nodes[i].bufY;
    }
}

void ModelClass::UpdateXmlWithScale()
{
    ModelXml->DeleteAttribute(wxT("offsetXpct"));
    ModelXml->DeleteAttribute(wxT("offsetYpct"));
    ModelXml->DeleteAttribute(wxT("PreviewScale"));
    ModelXml->AddAttribute(wxT("offsetXpct"), wxString::Format(wxT("%6.4f"),offsetXpct));
    ModelXml->AddAttribute(wxT("offsetYpct"), wxString::Format(wxT("%6.4f"),offsetYpct));
    ModelXml->AddAttribute(wxT("PreviewScale"), wxString::Format(wxT("%6.4f"),PreviewScale));
}

// display model using a single color
void ModelClass::DisplayModelOnWindow(wxWindow* window, const wxColour* color)
{
    size_t NodeCount=Nodes.size();
    wxCoord sx,sy;
    wxClientDC dc(window);
    wxPen pen;
    wxCoord w, h;

    dc.GetSize(&w, &h);
    double scale=RenderHt > RenderWi ? double(h) / RenderHt * PreviewScale : double(w) / RenderWi * PreviewScale;
    /*
    // this isn't an ideal scaling algorithm - room for improvement here
    double windowDiagonal=sqrt(w*w+h*h);
    double modelDiagonal=sqrt(RenderWi*RenderWi+RenderHt*RenderHt);
    double scale=windowDiagonal / modelDiagonal * PreviewScale;
    */
    dc.SetAxisOrientation(true,true);
    dc.SetDeviceOrigin(int(offsetXpct*w)+w/2,int(offsetYpct*h)+h-std::max((h-int(double(RenderHt-1)*scale))/2,1));
    dc.SetUserScale(scale,scale);

    pen.SetColour(*color);
    dc.SetPen(pen);
    for(size_t i=0; i<NodeCount; i++)
    {
        // draw node on screen
        sx=Nodes[i].screenX;
        sy=Nodes[i].screenY;
        dc.DrawPoint(sx,sy);
        //dc.DrawCircle(sx*factor,sy*factor,radius);
    }
}

// display model using colors stored in each node
void ModelClass::DisplayModelOnWindow(wxWindow* window)
{
    size_t NodeCount=Nodes.size();
    wxCoord sx,sy;
    wxClientDC dc(window);
    wxPen pen;
    wxColour color;
    wxCoord w, h;

    dc.GetSize(&w, &h);
    double scale=RenderHt > RenderWi ? double(h) / RenderHt * PreviewScale : double(w) / RenderWi * PreviewScale;
    /*
    // this isn't an ideal scaling algorithm - room for improvement here
    double windowDiagonal=sqrt(w*w+h*h);
    double modelDiagonal=sqrt(RenderWi*RenderWi+RenderHt*RenderHt);
    double scale=windowDiagonal / modelDiagonal * PreviewScale;
    */
    dc.SetAxisOrientation(true,true);
    dc.SetDeviceOrigin(int(offsetXpct*w)+w/2,int(offsetYpct*h)+h-std::max((h-int(double(RenderHt-1)*scale))/2,1));
    dc.SetUserScale(scale,scale);

    for(size_t i=0; i<NodeCount; i++)
    {
        // draw node on screen
        sx=Nodes[i].screenX;
        sy=Nodes[i].screenY;
        Nodes[i].GetColor(color);
        pen.SetColour(color);
        dc.SetPen(pen);
        dc.DrawPoint(sx,sy);
        //dc.DrawCircle(sx*factor,sy*factor,radius);
    }
}

// uses DrawCircle instead of DrawPoint
void ModelClass::DisplayEffectOnWindow(wxWindow* window)
{
    wxPen pen;
    wxBrush brush;
    wxClientDC dc(window);
    wxColour color;
    wxCoord w, h;
    dc.GetSize(&w, &h);
    double scaleX = double(w) / RenderWi;
    double scaleY = double(h) / RenderHt;
    double scale=scaleY < scaleX ? scaleY : scaleX;
    dc.SetAxisOrientation(true,true);
    dc.SetDeviceOrigin(w/2,h-std::max((h-int(double(RenderHt-1)*scale))/2,1));

    int radius=1;
    int factor=8;
    if (scale < 0.5)
    {
        radius=int(1.0/scale+0.5);
        factor=1;
    }
    else if (scale < 8.0)
    {
        factor=int(scale+0.5);
    }
    dc.SetUserScale(scale/factor,scale/factor);

    // if the radius/factor are not yielding good results, uncomment the next line
    //StatusBar1->SetStatusText(wxString::Format(wxT("Scale=%5.3f, radius=%d, factor=%d"),scale,radius,factor));

    /*
            // check that origin is in the right place
            dc.SetUserScale(4,4);
            color.Set(0,0,255);
            pen.SetColour(color);
            dc.SetPen(pen);
            dc.DrawPoint(0,0);
            dc.DrawPoint(1,1);
            dc.DrawPoint(2,2);
            return;
    */
    // layer calculation and map to output
    size_t NodeCount=Nodes.size();
    double sx,sy;

    for(size_t i=0; i<NodeCount; i++)
    {
        // draw node on screen
        Nodes[i].GetColor(color);
        pen.SetColour(color);
        brush.SetColour(color);
        brush.SetStyle(wxBRUSHSTYLE_SOLID);
        dc.SetPen(pen);
        dc.SetBrush(brush);
        sx=Nodes[i].screenX;
        sy=Nodes[i].screenY;
        //#     dc.DrawPoint(Nodes[i].screenX, Nodes[i].screenY);
        //dc.DrawPoint(sx,sy);
        dc.DrawCircle(sx*factor,sy*factor,radius);
    }
}
