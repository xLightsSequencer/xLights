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
#include <wx/graphics.h>
#include "xLightsMain.h" //for Preview and Other model collections

void ModelClass::SetFromXml(wxXmlNode* ModelNode, bool zeroBased)
{
    wxString tempstr,channelstr;
    wxString customModel,RGBorder;
    long degrees, StartChannel, channel;
    size_t i;

    ModelXml=ModelNode;
    TreeDegrees=0;
    StrobeRate=0;
    Nodes.clear();

    name=ModelNode->GetAttribute("name");
    DisplayAs=ModelNode->GetAttribute("DisplayAs");
    if (ModelNode->HasAttribute("StringType"))
    {
        // post 3.1.4
        StringType=ModelNode->GetAttribute("StringType");
    }
    else
    {
        // 3.1.4 and earlier
        StringType=ModelNode->GetAttribute("Order","RGB")+" Nodes";
    }
    SingleNode=HasSingleNode(StringType);
    SingleChannel=HasSingleChannel(StringType);
    RGBorder=SingleNode ? "RGB" : RGBorder=StringType.Left(3);
    rgbidx[0]=std::max(RGBorder.Find('R'),0);
    rgbidx[1]=std::max(RGBorder.Find('G'),0);
    rgbidx[2]=std::max(RGBorder.Find('B'),0);

    tempstr=ModelNode->GetAttribute("parm1");
    tempstr.ToLong(&parm1);
    tempstr=ModelNode->GetAttribute("parm2");
    tempstr.ToLong(&parm2);
    tempstr=ModelNode->GetAttribute("parm3");
    tempstr.ToLong(&parm3);
    tempstr=ModelNode->GetAttribute("StartChannel","1");
    tempstr.ToLong(&StartChannel);
    if (ModelNode->HasAttribute("ModelBrightness"))
    {
        tempstr=ModelNode->GetAttribute("ModelBrightness");
        tempstr.ToLong(&ModelBrightness);
    }
    tempstr=ModelNode->GetAttribute("Dir");
    IsLtoR=tempstr != "R";
    if (ModelNode->HasAttribute("StartSide"))
    {
        tempstr=ModelNode->GetAttribute("StartSide");
        isBotToTop = (tempstr == "B");
    }
    else
    {
        isBotToTop=true;
    }

    tempstr=ModelNode->GetAttribute("Antialias","0");
    tempstr.ToLong(&Antialias);
    AliasFactor=1 << Antialias;


    MyDisplay=IsMyDisplay(ModelNode);

    tempstr=ModelNode->GetAttribute("offsetXpct","0");
    tempstr.ToDouble(&offsetXpct);
    tempstr=ModelNode->GetAttribute("offsetYpct","0");
    tempstr.ToDouble(&offsetYpct);
    tempstr=ModelNode->GetAttribute("PreviewScale","0.333");
    tempstr.ToDouble(&PreviewScale);
    tempstr=ModelNode->GetAttribute("PreviewRotation","0");
    tempstr.ToLong(&degrees);
    PreviewRotation=degrees;

    // calculate starting channel numbers for each string
    size_t NumberOfStrings= HasOneString(DisplayAs) ? 1 : parm1;
    int ChannelsPerString=parm2*3;
    if (SingleChannel)
        ChannelsPerString=1;
    else if (SingleNode)
        ChannelsPerString=3;

    if (ModelNode->HasAttribute("CustomModel"))
    {
        customModel = ModelNode->GetAttribute("CustomModel");
        int maxval=GetCustomMaxChannel(customModel);
        // fix NumberOfStrings
        if (SingleNode)
        {
            NumberOfStrings=maxval;
        }
        else
        {
            ChannelsPerString=maxval*3;
        }
    }

    tempstr=ModelNode->GetAttribute("Advanced","0");
    bool HasIndividualStartChans=tempstr == "1";
    stringStartChan.clear();
    stringStartChan.resize(NumberOfStrings);
    for (i=0; i<NumberOfStrings; i++)
    {
        tempstr=StartChanAttrName(i);
        if (!zeroBased && HasIndividualStartChans && ModelNode->HasAttribute(tempstr))
        {
            ModelNode->GetAttribute(tempstr, &channelstr);
            channelstr.ToLong(&channel);
            stringStartChan[i] = channel-1;
        }
        else
        {
            stringStartChan[i] = (zeroBased? 0 : StartChannel-1) + i*ChannelsPerString;
        }
    }


    // initialize model based on the DisplayAs value
    wxStringTokenizer tkz(DisplayAs, " ");
    wxString token = tkz.GetNextToken();
    if (token == "Tree")
    {
        InitVMatrix();
        token = tkz.GetNextToken();
        token.ToLong(&degrees);
        SetTreeCoord(degrees);
    }
    else if (DisplayAs == "Custom")
    {
        InitCustomMatrix(customModel);
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == "Vert Matrix")
    {
        InitVMatrix();
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == "Horiz Matrix")
    {
        InitHMatrix();
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == "Single Line")
    {
        InitLine();
        SetLineCoord();
    }
    else if (DisplayAs == "Arches")
    {
        InitHMatrix(); // Old call was InitLine();
        SetArchCoord();
    }
    else if (DisplayAs == "Window Frame")
    {
        InitFrame();
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == "Star")
    {
        InitStar();
        CopyBufCoord2ScreenCoord();
    }
    else if (DisplayAs == "Wreath")
    {
        InitWreath();
        CopyBufCoord2ScreenCoord();
    }



    size_t NodeCount=GetNodeCount();
    for(size_t i=0; i<NodeCount; i++)
    {
        Nodes[i]->sparkle = rand() % 10000;
    }
}

void ModelClass::GetChanIntensity(size_t nodenum, wxByte chidx, size_t *absChNum, uint8_t *intensity)
{
    Nodes[nodenum]->GetChanIntensity(chidx,rgbidx[chidx],absChNum,intensity);
}

void ModelClass::SetChanIntensity(size_t nodenum, wxByte chidx, uint8_t intensity)
{
    Nodes[nodenum]->SetChanIntensity(rgbidx[chidx],intensity);
}

void ModelClass::SetChanIntensityAll(size_t nodenum, uint8_t intensity)
{
    Nodes[nodenum]->SetChanIntensityAll(intensity);
}

// only valid for rgb nodes and dumb strings (not traditional strings)
wxChar ModelClass::GetChannelColorLetter(wxByte chidx)
{
    wxString rgb="RGB";
    return rgb[rgbidx[chidx]];
}

int ModelClass::GetLastChannel()
{
    int LastChan=0;
    size_t NodeCount=GetNodeCount();
    for(size_t idx=0; idx<NodeCount; idx++)
    {
        LastChan=std::max(LastChan,Nodes[idx]->ActChan+2);
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

// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void ModelClass::InitVMatrix()
{
    int y,x,idx,stringnum,segmentnum,yincr;
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(PixelsPerStrand,NumStrands);
    SetNodeCount(parm1,PixelsPerString);
    SetRenderSize(PixelsPerStrand,NumStrands);

    // create output mapping
    if (SingleNode)
    {
        x=0;
        for (size_t n=0; n<Nodes.size(); n++)
        {
            Nodes[n]->ActChan = stringStartChan[n];
            y=0;
            yincr=1;
            for (size_t c=0; c<PixelsPerString; c++)
            {
                Nodes[n]->Coords[c].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[n]->Coords[c].bufY=y;
                y+=yincr;
                if (y < 0 || y >= PixelsPerStrand)
                {
                    yincr=-yincr;
                    y+=yincr;
                    x++;
                }
            }
        }
    }
    else
    {
        for (x=0; x < NumStrands; x++)
        {
            stringnum=x / parm3;
            segmentnum=x % parm3;
            for(y=0; y < PixelsPerStrand; y++)
            {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + y*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[idx]->Coords[0].bufY= isBotToTop == (segmentnum % 2 == 0) ? y:PixelsPerStrand-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}

// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void ModelClass::InitHMatrix()
{
    int y,x,idx,stringnum,segmentnum,xincr;
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(NumStrands,PixelsPerStrand);
    SetNodeCount(parm1,PixelsPerString);
    SetRenderSize(NumStrands,PixelsPerStrand);

    // create output mapping
    if (SingleNode)
    {
        y=0;
        for (size_t n=0; n<Nodes.size(); n++)
        {
            Nodes[n]->ActChan = stringStartChan[n];
            x=0;
            xincr=1;
            for (size_t c=0; c<PixelsPerString; c++)
            {
                Nodes[n]->Coords[c].bufX=x;
                Nodes[n]->Coords[c].bufY=isBotToTop ? y :NumStrands-y-1;
                x+=xincr;
                if (x < 0 || x >= PixelsPerStrand)
                {
                    xincr=-xincr;
                    x+=xincr;
                    y++;
                }
            }
        }
    }
    else
    {
        for (y=0; y < NumStrands; y++)
        {
            stringnum=y / parm3;
            segmentnum=y % parm3;
            for(x=0; x<PixelsPerStrand; x++)
            {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + x*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand-x-1 : x;
                Nodes[idx]->Coords[0].bufY= isBotToTop ? y :NumStrands-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}

int ModelClass::GetCustomMaxChannel(const wxString& customModel)
{
    wxString value;
    wxArrayString cols;
    long val,maxval=0;
    wxString valstr;

    wxArrayString rows=wxSplit(customModel,';');
    for(size_t row=0; row < rows.size(); row++)
    {
        cols=wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++)
        {
            valstr=cols[col];
            if (!valstr.IsEmpty() && valstr != "0")
            {
                valstr.ToLong(&val);
                maxval=std::max(val,maxval);
            }
        }
    }
    return maxval;
}

void ModelClass::InitCustomMatrix(const wxString& customModel)
{
    wxString value;
    wxArrayString cols;
    long idx;
    int width=1;
    std::vector<int> nodemap;

    wxArrayString rows=wxSplit(customModel,';');
    int height=rows.size();
    int cpn = ChannelsPerNode();
    for(size_t row=0; row < rows.size(); row++)
    {
        cols=wxSplit(rows[row],',');
        if (cols.size() > width) width=cols.size();
        for(size_t col=0; col < cols.size(); col++)
        {
            value=cols[col];
            if (!value.IsEmpty() && value != "0")
            {
                value.ToLong(&idx);

                // increase nodemap size if necessary
                if (idx > nodemap.size())
                {
                    nodemap.resize(idx, -1);
                }
                idx--;  // adjust to 0-based

                // is node already defined in map?
                if (nodemap[idx] < 0)
                {
                    // unmapped - so add a node
                    nodemap[idx]=Nodes.size();
                    SetNodeCount(1,0);  // this creates a node of the correct class
                    Nodes.back()->StringNum= SingleNode ? idx : 0;
                    Nodes.back()->ActChan=stringStartChan[0] + idx * cpn;
                    Nodes.back()->AddBufCoord(col,height - row - 1);
                }
                else
                {
                    // mapped - so add a coord to existing node
                    Nodes[nodemap[idx]]->AddBufCoord(col,height - row - 1);
                }

            }
        }
    }
    SetBufferSize(height,width);
}

double ModelClass::toRadians(long degrees)
{
    return 2.0*M_PI*double(degrees)/360.0;
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
    double radians=toRadians(degrees);
    double radius=RenderWi/2.0;
    double StartAngle=-radians/2.0;
    double AngleIncr=radians/double(BufferWi-1);
    //wxString msg=wxString::Format("BufferHt=%d, BufferWi=%d, factor=%d, RenderHt=%d, RenderWi=%d\n",BufferHt,BufferWi,factor,RenderHt,RenderWi);
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++)
    {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            bufferX=Nodes[n]->Coords[c].bufX;
            bufferY=Nodes[n]->Coords[c].bufY;
            angle=StartAngle + double(bufferX) * AngleIncr;
            x0=radius * sin(angle);
            Nodes[n]->Coords[c].screenX=floor(x0*(1.0-double(bufferY)/double(BufferHt)) + 0.5);
            Nodes[n]->Coords[c].screenY=bufferY * factor;
        }
    }
}

// initialize buffer coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void ModelClass::InitLine()
{
    SetNodeCount(parm1,parm2);
    SetBufferSize(1,parm2);
    int LastStringNum=-1;
    int chan,idx;
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++)
    {
        if (Nodes[n]->StringNum != LastStringNum)
        {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
            idx=0;
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            Nodes[n]->Coords[c].bufX=IsLtoR ? idx : parm2-idx-1;
            Nodes[n]->Coords[c].bufY=0;
            idx++;
        }
    }
}

// parm3 is number of points
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
void ModelClass::InitStar()
{
    if (parm3 < 2) parm3=2; // need at least 2 arms
    SetNodeCount(parm1,parm2);
    int numlights=parm1*parm2;
    SetBufferSize(numlights+1,numlights+1);
    int LastStringNum=-1;
    int chan,cursegment,nextsegment,x,y;
    int offset=numlights/2;
    int numsegments=parm3*2;
    double segstart_x,segstart_y,segend_x,segend_y,segstart_pct,segend_pct,r,segpct,dseg;
    double dpct=1.0/(double)numsegments;
    double OuterRadius=offset;
    double InnerRadius=OuterRadius/2.618034;    // divide by golden ratio squared
    double pct=isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
    double pctIncr=1.0 / (double)numlights;     // this is cw
    if (IsLtoR != isBotToTop) pctIncr*=-1.0;    // adjust to ccw
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++)
    {
        if (Nodes[n]->StringNum != LastStringNum)
        {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            cursegment=(int)((double)numsegments*pct) % numsegments;
            nextsegment=(cursegment+1) % numsegments;
            segstart_pct=(double)cursegment / numsegments;
            segend_pct=(double)nextsegment / numsegments;
            dseg=pct - segstart_pct;
            segpct=dseg / dpct;
            r=cursegment%2==0 ? OuterRadius : InnerRadius;
            segstart_x=r*sin(segstart_pct*2.0*M_PI);
            segstart_y=r*cos(segstart_pct*2.0*M_PI);
            r=nextsegment%2==0 ? OuterRadius : InnerRadius;
            segend_x=r*sin(segend_pct*2.0*M_PI);
            segend_y=r*cos(segend_pct*2.0*M_PI);
            // now interpolate between segstart and segend
            x=(segend_x - segstart_x)*segpct + segstart_x + offset + 0.5;
            y=(segend_y - segstart_y)*segpct + segstart_y + offset + 0.5;
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            pct+=pctIncr;
            if (pct >= 1.0) pct-=1.0;
            if (pct < 0.0) pct+=1.0;
        }
    }
}

// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw
void ModelClass::InitWreath()
{
    SetNodeCount(parm1,parm2);
    int numlights=parm1*parm2;
    SetBufferSize(numlights+1,numlights+1);
    int LastStringNum=-1;
    int offset=numlights/2;
    double r=offset;
    int chan,x,y;
    double pct=isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
    double pctIncr=1.0 / (double)numlights;     // this is cw
    if (IsLtoR != isBotToTop) pctIncr*=-1.0;    // adjust to ccw
    int ChanIncr=SingleChannel ?  1 : 3;
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++)
    {
        if (Nodes[n]->StringNum != LastStringNum)
        {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            x=r*sin(pct*2.0*M_PI) + offset + 0.5;
            y=r*cos(pct*2.0*M_PI) + offset + 0.5;
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            pct+=pctIncr;
            if (pct >= 1.0) pct-=1.0;
            if (pct < 0.0) pct+=1.0;
        }
    }
}

// initialize screen coordinates
// parm1=Number of Strings/Arches
// parm2=Pixels Per String/Arch
void ModelClass::SetLineCoord()
{
    int x,y;
    int idx=0;
    size_t NodeCount=GetNodeCount();
    int numlights=parm1*parm2;
    int half=numlights/2;
    SetRenderSize(numlights*2,numlights);
    double radians=toRadians(PreviewRotation);
    for(size_t n=0; n<NodeCount; n++)
    {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            x=cos(radians)*idx;
            x=IsLtoR ? x - half : half - x;
            y=sin(radians)*idx;
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y + numlights;
            idx++;
        }
    }
}

// Set screen coordinates for arches
void ModelClass::SetArchCoord()
{
    int xoffset,x,y;
    int numlights=parm1*parm2;
    size_t NodeCount=GetNodeCount();
    SetRenderSize(parm2,numlights*2);
    double midpt=parm2;
    for(size_t n=0; n<NodeCount; n++)
    {
        xoffset=Nodes[n]->StringNum*parm2*2 - numlights;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            double angle=-M_PI/2.0 + M_PI * (double)Nodes[n]->Coords[c].bufX/midpt;
            x=xoffset + (int)floor(midpt*sin(angle)+midpt);
            y=(int)floor(midpt*cos(angle)+0.5);
            Nodes[n]->Coords[c].screenX=x;
            Nodes[n]->Coords[c].screenY=y;
        }
    }
}

// initialize buffer coordinates
// parm1=Nodes on Top
// parm2=Nodes left and right
// parm3=Nodes on Bottom
void ModelClass::InitFrame()
{
    int x,y,newx,newy;
    SetNodeCount(1,parm1+2*parm2+parm3);
    int FrameWidth=std::max(parm1,parm3)+2;
    SetBufferSize(parm2,FrameWidth);   // treat as outside of matrix
    //SetBufferSize(1,Nodes.size());   // treat as single string
    SetRenderSize(parm2,FrameWidth);
    int chan=stringStartChan[0];
    int ChanIncr=SingleChannel ?  1 : 3;

    int xincr[4]= {0,1,0,-1}; // indexed by side
    int yincr[4]= {1,0,-1,0};
    x=IsLtoR ? 0 : FrameWidth-1;
    y=isBotToTop ? 0 : parm2-1;
    int dir=1;            // 1=clockwise
    int side=x>0 ? 2 : 0; // 0=left, 1=top, 2=right, 3=bottom
    int SideIncr=1;       // 1=clockwise
    if ((parm1 > parm3 && x>0) || (parm3 > parm1 && x==0))
    {
        // counter-clockwise
        dir=-1;
        SideIncr=3;
    }

    // determine starting position
    if (parm1 > parm3)
    {
        // more nodes on top, must start at bottom
        y=0;
    }
    else if (parm3 > parm1)
    {
        // more nodes on bottom, must start at top
        y=parm2-1;
    }
    else
    {
        // equal top and bottom, can start in any corner
        // assume clockwise numbering
        if (x>0 && y==0)
        {
            // starting in lower right
            side=3;
        }
        else if (x==0 && y>0)
        {
            // starting in upper left
            side=1;
        }
    }

    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++)
    {
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            Nodes[n]->Coords[c].bufX=x;
            Nodes[n]->Coords[c].bufY=y;
            newx=x+xincr[side]*dir;
            newy=y+yincr[side]*dir;
            if (newx < 0 || newx >= FrameWidth || newy < 0 || newy >= parm2)
            {
                // move to the next side
                side=(side+SideIncr) % 4;
                newx=x+xincr[side]*dir;
                newy=y+yincr[side]*dir;
            }
            x=newx;
            y=newy;
        }
    }
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

// not valid for Frame or Custom
int ModelClass::NodesPerString()
{
    return SingleNode ? 1 : parm2;
}

int ModelClass::NodeStartChannel(size_t nodenum)
{
    return Nodes[nodenum]->ActChan;
}

int ModelClass::ChannelsPerNode()
{
    return SingleChannel ? 1 : 3;
}

// set size of Nodes vector and each Node's Coords vector
void ModelClass::SetNodeCount(size_t NumStrings, size_t NodesPerString)
{
    size_t n;
    if (SingleNode)
    {
        if (StringType=="Single Color Red")
        {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassRed(n,NodesPerString)));
        }
        else if (StringType=="Single Color Green")
        {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassGreen(n,NodesPerString)));
        }
        else if (StringType=="Single Color Blue")
        {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassBlue(n,NodesPerString)));
        }
        else if (StringType=="Single Color White")
        {
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n,NodesPerString)));
        }
        else if (StringType=="Strobes White 3fps")
        {
            StrobeRate=7;  // 1 out of every 7 frames
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeClassWhite(n,NodesPerString)));
        }
        else
        {
            // 3 Channel RGB
            for(n=0; n<NumStrings; n++)
                Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n,NodesPerString)));
        }
    }
    else if (NodesPerString == 0)
    {
        Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(0, 0)));
    }
    else
    {
        size_t numnodes=NumStrings*NodesPerString;
        for(n=0; n<numnodes; n++)
            Nodes.push_back(NodeBaseClassPtr(new NodeBaseClass(n/NodesPerString, 1)));
    }
}

bool ModelClass::CanRotate()
{
    return DisplayAs == "Single Line";
}

void ModelClass::Rotate(int degrees)
{
    if (!CanRotate()) return;
    PreviewRotation=degrees;
    SetLineCoord();
}

int ModelClass::GetRotation()
{
    return PreviewRotation;
}


// returns a number where the first node is 1
int ModelClass::GetNodeNumber(size_t nodenum)
{
    if (nodenum >= Nodes.size()) return 0;
    //if (Nodes[nodenum].bufX < 0) return 0;
    int sn=Nodes[nodenum]->StringNum;
    return (Nodes[nodenum]->ActChan - stringStartChan[sn]) / 3 + sn*NodesPerString() + 1;
}

size_t ModelClass::GetNodeCount()
{
    return Nodes.size();
}

int ModelClass::GetChanCount()
{
    size_t NodeCnt=GetNodeCount();
    return NodeCnt==0 ? 0 : NodeCnt * Nodes[0]->GetChanCount();
}
size_t ModelClass::GetCoordCount(size_t nodenum)
{
    return nodenum < Nodes.size() ? Nodes[nodenum]->Coords.size() : 0;
}

#if 0 //obsolete
int ModelClass::FindChannelAt(int x, int y)
{
    size_t NodeCount=GetNodeCount();
    for(size_t n=0; n<NodeCount; n++)
    {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
//??            if ((Nodes[n]->Coords[c].screenX == x) && (Nodes[n]->Coords[c].screenY == y)) return Nodes[n].ActChan;
            if ((Nodes[n]->Coords[c].bufX == x) && (Nodes[n]->Coords[c].bufY == y)) return Nodes[n]->ActChan;
        }
    }
    return -1; //not found
}
#endif // 0


//return (x,y) matrix of channel#s for pgo RenderFaces:
#if 0 //obsolete
wxSize ModelClass::GetChannelCoords(std::vector<std::vector<int>>& chxy, bool shrink)
{
    size_t h = 0;
    if (shrink) chxy.clear();
    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n < NodeCount; n++)
    {
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++)
        {
//??            if ((Nodes[n]->Coords[c].screenX == x) && (Nodes[n]->Coords[c].screenY == y)) return Nodes[n].ActChan;
            if (Nodes[n]->Coords[c].bufX >= chxy.size()) chxy.resize(Nodes[n]->Coords[c].bufX + 1); //enlarge to fit; TODO: pad with -1s?
            std::vector<int>& row = chxy[Nodes[n]->Coords[c].bufX];
            if (Nodes[n]->Coords[c].bufY >= row.size()) row.resize(Nodes[n]->Coords[c].bufY + 1); //enlarge to fit; TODO: pad with -1s?
            if (Nodes[n]->Coords[c].bufY >= h) h = Nodes[n]->Coords[c].bufY + 1;
//            row[Nodes[n]->Coords[c].bufY] = Nodes[n]->ActChan;
            //GetNodeNumber(i)
        }
    }
    for (auto it = chxy.begin(); it != chxy.end(); ++it) //force rectangular matrix
        (*it).resize(h); //TODO: pad with -1s?
    return wxSize(chxy.size(), h); //tell caller how big the model is
}
#else
ModelClass* ModelClass::FindModel(const wxString& name)
{
//TODO: use static member array rather than xLightsFrame?
//first check active models:
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        if ((*it)->name == name) return &**it;
    }
//also check non-preview models:
    for (auto it = xLightsFrame::OtherModels.begin(); it != xLightsFrame::OtherModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        if ((*it)->name == name) return &**it;
    }
    return 0; //not found
}

size_t ModelClass::EnumModels(wxChoice* choices, const wxString& InactivePrefix)
{
//TODO: use static member array rather than xLightsFrame?
    size_t svcount = choices->GetCount(); //don't clear it; caller might have extra values in list
//first check active models:
    for (auto it = xLightsFrame::PreviewModels.begin(); it != xLightsFrame::PreviewModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        choices->Append((*it)->name);
    }
//also list non-preview models:
    for (auto it = xLightsFrame::OtherModels.begin(); it != xLightsFrame::OtherModels.end(); ++it)
    {
        if ((*it)->name.IsEmpty()) continue;
        choices->Append(InactivePrefix + (*it)->name); //show indicator for non-active models
    }
    return choices->GetCount() - svcount; //#entries added
}

bool ModelClass::IsCustom(void)
{
    return (DisplayAs == "Custom");
}

//convert # to AA format so it matches Custom Model grid display:
//this makes it *so* much easier to visually compare with Custom Model grid display
static wxString AA(int x)
{
    wxString retval;
    --x;
    if (x >= 26 * 26) { retval += 'A' + x / (26 * 26); x %= 26 * 26; }
    if (x >= 26) { retval += 'A' + x / 26; x %= 26; }
    retval += 'A' + x;
    return retval;
}
//add just the node#s to a choice list:
//NO add parsed info to choice list or check list box:
size_t ModelClass::GetChannelCoords(wxChoice* choices1, wxCheckListBox* choices2, wxListBox* choices3)
{
    if (choices1) choices1->Clear();
    if (choices2) choices2->Clear();
    if (choices3) choices3->Clear();
    if (choices1) choices1->Append(wxT("0: (none)"));
    if (choices2) choices2->Append(wxT("0: (none)"));
    if (choices3) choices3->Append(wxT("0: (none)"));
    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n < NodeCount; n++)
    {
        wxString newstr;
//        debug(10, "model::node[%d/%d]: #coords %d, ach# %d, str %d", n, NodeCount, Nodes[n]->Coords.size(), Nodes[n]->StringNum, Nodes[n]->ActChan);
        if (Nodes[n]->Coords.empty()) continue;
#if 0
        if (GetCoordCount(n) > 1) //show count and first + last coordinates
            if (IsCustom())
                newstr = wxString::Format(wxT("%d: %d# @%s%d-%s%d"), GetNodeNumber(n), GetCoordCount(n), AA(Nodes[n]->Coords.front().bufX + 1), BufferHt - Nodes[n]->Coords.front().bufY, AA(Nodes[n]->Coords.back().bufX + 1), BufferHt - Nodes[n]->Coords.back().bufY); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
            else
                newstr = wxString::Format(wxT("%d: %d# @(%d,%d)-(%d,%d"), GetNodeNumber(n), GetCoordCount(n), Nodes[n]->Coords.front().bufX + 1, BufferHt - Nodes[n]->Coords.front().bufY, Nodes[n]->Coords.back().bufX + 1, BufferHt - Nodes[n]->Coords.back().bufY); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else //just show singleton
            if (IsCustom())
                newstr = wxString::Format(wxT("%d: @%s%d"), GetNodeNumber(n), AA(Nodes[n]->Coords.front().bufX + 1), BufferHt - Nodes[n]->Coords.front().bufY);
            else
                newstr = wxString::Format(wxT("%d: @(%d,%d)"), GetNodeNumber(n), Nodes[n]->Coords.front().bufX + 1, BufferHt - Nodes[n]->Coords.front().bufY);
#else
        newstr = wxString::Format(wxT("%d"), GetNodeNumber(n));
#endif // 0
        if (choices1) choices1->Append(newstr);
        if (choices2) choices2->Append(newstr);
        if (choices3)
        {
            wxArrayString strary;
            strary.Add(newstr);
            choices3->InsertItems(strary, choices3->GetCount() + 0);
        }
#if 0
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + y*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[idx]->Coords[0].bufY= isBotToTop == (segmentnum % 2 == 0) ? y:PixelsPerStrand-y-1;
                Nodes[idx]->StringNum=stringnum;
#endif // 0
    }
    return (choices1? choices1->GetCount(): 0) + (choices2? choices2->GetCount(): 0);
}
//get parsed node info:
wxString ModelClass::GetNodeXY(int node)
{
    if ((node < 0) || (node >= GetNodeCount())) return wxEmptyString;
    if (Nodes[node]->Coords.empty()) return wxEmptyString;
    if (GetCoordCount(node) > 1) //show count and first + last coordinates
        if (IsCustom())
            return wxString::Format(wxT("%d: %d# @%s%d-%s%d"), GetNodeNumber(node), GetCoordCount(node), AA(Nodes[node]->Coords.front().bufX + 1), BufferHt - Nodes[node]->Coords.front().bufY, AA(Nodes[node]->Coords.back().bufX + 1), BufferHt - Nodes[node]->Coords.back().bufY); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
        else
            return wxString::Format(wxT("%d: %d# @(%d,%d)-(%d,%d"), GetNodeNumber(node), GetCoordCount(node), Nodes[node]->Coords.front().bufX + 1, BufferHt - Nodes[node]->Coords.front().bufY, Nodes[node]->Coords.back().bufX + 1, BufferHt - Nodes[node]->Coords.back().bufY); //NOTE: only need first (X,Y) for each channel, but show last and count as well; Y is in reverse order
    else //just show singleton
        if (IsCustom())
            return wxString::Format(wxT("%d: @%s%d"), GetNodeNumber(node), AA(Nodes[node]->Coords.front().bufX + 1), BufferHt - Nodes[node]->Coords.front().bufY);
        else
            return wxString::Format(wxT("%d: @(%d,%d)"), GetNodeNumber(node), Nodes[node]->Coords.front().bufX + 1, BufferHt - Nodes[node]->Coords.front().bufY);
}

//extract first (X,Y) from string formatted above:
bool ModelClass::ParseFaceElement(const wxString& str, wxPoint* first_xy)
{
    first_xy->x = first_xy->y = 0;
    if (str.Find('@') == wxNOT_FOUND) return false;
#if 0 //hard-coded test results
    first_xy->x = 1; first_xy->y = 2; //TODO
#else
    wxString xystr = str.AfterFirst('@');
    if (xystr.empty()) return false;
    if (xystr[0] == '(')
    {
        long val;
        xystr.Remove(0, 1);
        if (!xystr.BeforeFirst(',').ToLong(&val)) return false;
        first_xy->x = val;
        if (!xystr.AfterFirst(',').BeforeFirst(')').ToLong(&val)) return false;
        first_xy->y = val;
    }
    else
    {
        int parts = 0;
        while (!xystr.empty() && (xystr[0] >= 'A') && (xystr[0] <= 'Z'))
        {
            first_xy->x *= 26;
            first_xy->x += xystr[0] - 'A';
            xystr.Remove(0, 1);
            parts |= 1;
        }
        while (!xystr.empty() && (xystr[0] >= '0') && (xystr[0] <= '9'))
        {
            first_xy->y *= 10;
            first_xy->y += xystr[0] - '0';
            xystr.Remove(0, 1);
            parts |= 2;
        }
        if (parts != 3) return false;
        if (!xystr.empty() && (xystr[0] != '-')) return false;
    }
#endif // 0
    return true;
}
#endif // 0

wxString ModelClass::ChannelLayoutHtml()
{
    size_t NodeCount=GetNodeCount();
    size_t i,idx;
    int n,x,y,s;
    wxString bgcolor;
    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    bool IsCustom = DisplayAs == "Custom";
    wxString direction;
    if (IsCustom)
    {
        direction="n/a";
    }
    else if (!IsLtoR)
    {
        if(!isBotToTop)
            direction="Top Right";
        else
            direction="Bottom Right";
    }
    else
    {
        if (!isBotToTop)
            direction="Top Left";
        else
            direction="Bottom Left";
    }

    wxString html = "<html><body><table border=0>";
    html+="<tr><td>Name:</td><td>"+name+"</td></tr>";
    html+="<tr><td>Display As:</td><td>"+DisplayAs+"</td></tr>";
    html+="<tr><td>String Type:</td><td>"+StringType+"</td></tr>";
    html+="<tr><td>Start Corner:</td><td>"+direction+"</td></tr>";
    html+=wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>",NodeCount);
    html+=wxString::Format("<tr><td>Height:</td><td>%d</td></tr>",BufferHt);
    html+="</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";


  int Ibufx,Ibufy;

     if (BufferHt == 1)
    {
        // single line or arch
        html+="<tr>";
        for(i=1; i<=NodeCount; i++)
        {
            n=IsLtoR ? i : NodeCount-i+1;
            s=Nodes[n-1]->StringNum+1;
            bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
            html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
        }
        html+="</tr>";
    }
    else if (BufferHt > 1)
    {
        // horizontal or vertical matrix or frame
        for(i=0; i<NodeCount; i++)
        {

            Ibufx = Nodes[i]->Coords[0].bufX;
            Ibufy = Nodes[i]->Coords[0].bufY;
            idx=Nodes[i]->Coords[0].bufY * BufferWi + Nodes[i]->Coords[0].bufX;
            if (idx < chmap.size()) chmap[idx]=GetNodeNumber(i);
        }
        for(y=BufferHt-1; y>=0; y--)
        {
            html+="<tr>";
            for(x=0; x<BufferWi; x++)
            {
                n=chmap[y*BufferWi+x];
                if (n==0)
                {
                    html+="<td></td>";
                }
                else
                {
                    s=Nodes[n-1]->StringNum+1;
                    bgcolor=s%2 == 1 ? "#ADD8E6" : "#90EE90";
                    html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%ds%d</td>",n,s);
                }
            }
            html+="</tr>";
        }
    }
    else
    {
        html+="<tr><td>Error - invalid height</td></tr>";
    }
    html+="</table></body></html>";
    return html;
}


// initialize screen coordinates
void ModelClass::CopyBufCoord2ScreenCoord()
{
    size_t NodeCount=GetNodeCount();
    int xoffset=BufferWi/2;
    for(size_t n=0; n<NodeCount; n++)
    {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            Nodes[n]->Coords[c].screenX = Nodes[n]->Coords[c].bufX - xoffset;
            Nodes[n]->Coords[c].screenY = Nodes[n]->Coords[c].bufY;
        }
    }
    SetRenderSize(BufferHt,BufferWi);
}

void ModelClass::UpdateXmlWithScale()
{
    ModelXml->DeleteAttribute("offsetXpct");
    ModelXml->DeleteAttribute("offsetYpct");
    ModelXml->DeleteAttribute("PreviewScale");
    ModelXml->DeleteAttribute("PreviewRotation");
    ModelXml->AddAttribute("offsetXpct", wxString::Format("%6.4f",offsetXpct));
    ModelXml->AddAttribute("offsetYpct", wxString::Format("%6.4f",offsetYpct));
    ModelXml->AddAttribute("PreviewScale", wxString::Format("%6.4f",PreviewScale));
    ModelXml->AddAttribute("PreviewRotation", wxString::Format("%d",PreviewRotation));
}


#ifdef __WXOSX__
class ModelGraphics
{
public:
    ModelGraphics(wxWindow *window) : lastColor(*wxBLACK)
    {
        gc = wxGraphicsContext::Create(window);
        gc->SetAntialiasMode(wxANTIALIAS_NONE);
        gc->Scale(1, -1);
        path = gc->CreatePath();
        pen.SetColour(lastColor);
        gc->SetPen(pen);
        brush.SetStyle(wxBRUSHSTYLE_SOLID);
        brush.SetColour(lastColor);
        gc->SetBrush(brush);
    }
    ~ModelGraphics()
    {
        gc->DrawPath(path);
        gc->Flush();
        delete gc;
    }
    void Translate(wxDouble x, wxDouble y)
    {
        gc->Translate(x, y);
    }
    void GetSize(wxDouble *x, wxDouble *y)
    {
        gc->GetSize(x, y);
    }

    void AddSquare(const wxColour &color, wxDouble x, wxDouble y, double size)
    {
        if (lastColor != color)
        {
            flush(color);
        }
        path.AddRectangle(x, y, size, size);
    }
    void AddCircle(const wxColour &color, wxDouble x, wxDouble y, double diameter)
    {
        if (lastColor != color)
        {
            flush(color);
        }
        path.AddEllipse(x, y, diameter, diameter);
    }

private:
    void flush(const wxColour &color)
    {
        gc->DrawPath(path);
        path = gc->CreatePath();
        lastColor = color;
        pen.SetColour(lastColor);
        gc->SetPen(pen);
        brush.SetColour(lastColor);
        gc->SetBrush(brush);
    }
    wxBrush brush;
    wxPen pen;
    wxGraphicsContext *gc;
    wxColor lastColor;
    wxGraphicsPath path;
};

#else
class ModelGraphics
{
public:
    ModelGraphics(wxWindow *window) : dc(window), lastColor(*wxRED)
    {
        dc.SetAxisOrientation(true,true);
        //dc.SetLogicalScale(1.0, -1.0);
        pen.SetColour(lastColor);
        dc.SetPen(pen);
        brush.SetStyle(wxBRUSHSTYLE_SOLID);
        brush.SetColour(lastColor);
        dc.SetBrush(brush);
    }
    ~ModelGraphics()
    {
    }
    void Translate(wxDouble x, wxDouble y)
    {
        dc.SetDeviceOrigin(x, -y);
    }
    void GetSize(wxDouble *x, wxDouble *y)
    {
        int x2, y2;
        dc.GetSize(&x2, &y2);
        *x = int(x2);
        *y = int(y2);
    }

    void AddSquare(const wxColour &color, wxDouble x, wxDouble y, double size)
    {
        if (lastColor != color)
        {
            flush(color);
        }
        if (size < 2)
        {
            size = 2;
        }
        dc.DrawRectangle(x,y,size,size);
    }
    void AddCircle(const wxColour &color, wxDouble x, wxDouble y, double diameter)
    {
        if (lastColor != color)
        {
            flush(color);
        }
        if (diameter < 2)
        {
            diameter = 2;
        }
        dc.DrawEllipse(x - (diameter/2), y - (diameter / 2), diameter, diameter);
    }

private:
    void flush(const wxColour &color)
    {
        lastColor = color;
        pen.SetColour(lastColor);
        dc.SetPen(pen);
        brush.SetColour(lastColor);
        dc.SetBrush(brush);
    }
    wxBrush brush;
    wxPen pen;
    wxColor lastColor;
    wxClientDC dc;
};

#endif


// display model using a single color
void ModelClass::DisplayModelOnWindow(wxWindow* window, const wxColour* color)
{
    size_t NodeCount=Nodes.size();
    wxCoord sx,sy;
    wxPen pen;
    wxDouble w, h;
    ModelGraphics gc(window);

    /*
    // this isn't an ideal scaling algorithm - room for improvement here
    double windowDiagonal=sqrt(w*w+h*h);
    double modelDiagonal=sqrt(RenderWi*RenderWi+RenderHt*RenderHt);
    double scale=windowDiagonal / modelDiagonal * PreviewScale;
    */

    gc.GetSize(&w, &h);
    double scale=RenderHt > RenderWi ? double(h) / RenderHt * PreviewScale : double(w) / RenderWi * PreviewScale;
    double scrx,scry;
    gc.Translate(int(offsetXpct*w)+w/2,
                 -(int(offsetYpct*h)+h-
                   std::max((int(h)-int(double(RenderHt-1)*scale))/2,1)));

    for(size_t n=0; n<NodeCount; n++)
    {
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            // draw node on screen
            sx=Nodes[n]->Coords[c].screenX;
            sy=Nodes[n]->Coords[c].screenY;
            gc.AddSquare(*color,sx*scale,sy*scale,0.0);
            scrx = sx*scale;
            scry = sy*scale;
            //      StatusBar1->SetStatusText(_("Status: DisplayModelOnWindow " )+wxString::Format(" x=%5ld y=%5ld ",scrx,scry));
        }
    }
}

// display model using colors stored in each node
// used when preview is running
void ModelClass::DisplayModelOnWindow(wxWindow* window)
{
    size_t NodeCount=Nodes.size();
    wxCoord sx,sy;
    wxPen pen;
    wxColour color;
    wxDouble w, h;
    ModelGraphics gc(window);

    /*
    // this isn't an ideal scaling algorithm - room for improvement here
    double windowDiagonal=sqrt(w*w+h*h);
    double modelDiagonal=sqrt(RenderWi*RenderWi+RenderHt*RenderHt);
    double scale=windowDiagonal / modelDiagonal * PreviewScale;
    */

    gc.GetSize(&w, &h);
    double scale=RenderHt > RenderWi ? double(h) / RenderHt * PreviewScale : double(w) / RenderWi * PreviewScale;
    gc.Translate(int(offsetXpct*w)+w/2,
                 -(int(offsetYpct*h)+h-
                   std::max((int(h)-int(double(RenderHt-1)*scale))/2,1)));

    // avoid performing StrobeRate test in inner loop for performance reasons
    if (StrobeRate==0)
    {
        // no strobing
        for(size_t n=0; n<NodeCount; n++)
        {
            Nodes[n]->GetColor(color);
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++)
            {
                // draw node on screen
                sx=Nodes[n]->Coords[c].screenX;
                sy=Nodes[n]->Coords[c].screenY;
                gc.AddSquare(color,sx*scale,sy*scale,0.0);
            }
        }
    }
    else
    {
        // flash individual nodes according to StrobeRate
        for(size_t n=0; n<NodeCount; n++)
        {
            Nodes[n]->GetColor(color);
            bool CanFlash = color.GetRGB() ==  0x00ffffff;
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++)
            {
                wxColor c2 = *wxBLACK;
                // draw node on screen
                if (CanFlash && rand() % StrobeRate == 0)
                {
                    c2 = color;
                }

                sx=Nodes[n]->Coords[c].screenX;
                sy=Nodes[n]->Coords[c].screenY;
                gc.AddSquare(c2,sx*scale,sy*scale,0.0);
            }
        }
    }
}

// uses DrawCircle instead of DrawPoint
void ModelClass::DisplayEffectOnWindow(wxWindow* window)
{
    wxColour color;
    wxDouble w, h;

    ModelGraphics gc(window);
    gc.GetSize(&w, &h);

    double scaleX = double(w) * 0.95 / RenderWi;
    double scaleY = double(h) * 0.95 / RenderHt;
    double scale=scaleY < scaleX ? scaleY : scaleX;

    gc.Translate(w/2,-int(double(RenderHt)*scale + double(RenderHt)*0.025*scale));

    double radius = scale/2.0;
    if (radius < 0.5)
    {
        radius = 0.5;
    }

    /*
    // check that origin is in the right place
    color.Set(0,0,255);
    gc.AddCircle(color, 0,0,1);
    gc.AddCircle(color, 1,1,1);
    gc.AddCircle(color, 2,2,1);
    gc.AddCircle(color, 3,3,1);
    gc.AddCircle(color, 4,4,1);
     */

    // layer calculation and map to output
    size_t NodeCount=Nodes.size();
    double sx,sy;
    for(size_t n=0; n<NodeCount; n++)
    {
        Nodes[n]->GetColor(color);
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++)
        {
            // draw node on screen
            sx=Nodes[n]->Coords[c].screenX;
            sy=Nodes[n]->Coords[c].screenY;
            //#     dc.DrawPoint(Nodes[i].screenX, Nodes[i].screenY);
            //dc.DrawPoint(sx,sy);
            //dc.DrawCircle(sx*scale,sy*scale,radius);
            //gc->DrawEllipse(sx*scale,sy*scale,radius,radius);
            gc.AddCircle(color, sx*scale,sy*scale,radius);
        }
    }
}
