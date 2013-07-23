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
    long degrees;
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
    tempstr=ModelNode->GetAttribute(wxT("Antialias"),wxT("0"));
    tempstr.ToLong(&Antialias);
    AliasFactor=1 << Antialias;
    MyDisplay=(ModelNode->GetAttribute(wxT("MyDisplay"),wxT("0")) == wxT("1"));

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
        stringStartChan[stringNum] = val;
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
            Nodes[idx].ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand + y;
            Nodes[idx].bufX=IsLtoR ? x : NumStrands-x-1;
            Nodes[idx].bufY=(segmentnum % 2 == 0) ? y : PixelsPerStrand-y-1;
            Nodes[idx].StringNum=stringnum;
        }
    }
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
    double radians=2.0*PI*double(degrees)/360.0;
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
            Nodes[idx].ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand + x;
            Nodes[idx].bufX=IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand-x-1 : x;
            Nodes[idx].bufY=y;
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
            Nodes[idx].ActChan = idx * 3;
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
    int xoffset=RenderWi/2;
    int idx=0;
    SetRenderSize(1,NodeCount);
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
    double AngleIncr=PI / parm2;
    for (ns=0; ns < parm1; ns++)
    {
        angle=-1.0*PI/2.0;
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
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across top
        y=parm2-1;
        for(x=0; x<parm1; x++)
        {
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=IsLtoR ? x+1 : parm1-x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // down side 2
        x=IsLtoR ? FrameWidth-1 : 0;
        for(y=parm2-1; y>=0; y--)
        {
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across bottom
        y=0;
        for(x=0; x<parm3; x++)
        {
            Nodes[idx].ActChan = idx * 3;
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
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across bottom
        y=0;
        for(x=0; x<parm3; x++)
        {
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=IsLtoR ? x+1: parm3-x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // up side 2
        x=IsLtoR ? FrameWidth-1 : 0;
        for(y=0; y<parm2; y++)
        {
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=x;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
        // across top
        y=parm2-1;
        for(x=0; x<parm1; x++)
        {
            Nodes[idx].ActChan = idx * 3;
            Nodes[idx].bufX=IsLtoR ? parm3-x : x+1;
            Nodes[idx].bufY=y;
            Nodes[idx].StringNum=0;
            idx++;
        }
    }
    SetBufferSize(parm2,FrameWidth);
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
