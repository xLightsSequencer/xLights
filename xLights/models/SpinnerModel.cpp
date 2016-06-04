#include "SpinnerModel.h"
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include "ModelScreenLocation.h"
SpinnerModel::SpinnerModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}
SpinnerModel::SpinnerModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    //ctor
}

SpinnerModel::~SpinnerModel()
{
    //dtor
}

static wxPGChoices SPINNER_STYLES;

void SpinnerModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    if (SPINNER_STYLES.GetCount() == 0) {
        SPINNER_STYLES.Add("Hollow Center");
        SPINNER_STYLES.Add("First Node Center");
        SPINNER_STYLES.Add("Last Node Center");
    }
    
    AddStyleProperties(grid);

    wxPGProperty *p = grid->Append(new wxUIntProperty("Center Nodes", "SpinnerCenterNodeCount", parm1));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    wxPGProperty *p = grid->Append(new wxUIntProperty("Arm Nodes", "SpinnerArmNodeCount", parm1));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 200);
    p->SetEditor("SpinCtrl");

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "SpinnerStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");


    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "MatrixStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");
    
    p = grid->Append(new wxUIntProperty("Lights/String", "MatrixLightCount", parm2));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Strands/String", "MatrixStrandCount", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");
    
    p = grid->Append(new wxEnumProperty("Starting Location", "MatrixStart", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));
}
void MatrixModel::AddStyleProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxEnumProperty("Direction", "MatrixStyle", MATRIX_STYLES, vMatrix ? 1 : 0));
}


int MatrixModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("MatrixStyle" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DisplayAs");
        ModelXml->AddAttribute("DisplayAs", event.GetPropertyValue().GetLong() ? "Vert Matrix" : "Horiz Matrix");
        SetFromXml(ModelXml, zeroBased);
        AdjustStringProperties(grid, parm1);
        return 3;
    } else if ("MatrixStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        AdjustStringProperties(grid, parm1);
        return 3 | 0x0008;
    } else if ("MatrixLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("MatrixStrandCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3 | 0x0008;
    } else if ("MatrixStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }
    
    return Model::OnPropertyGridChange(grid, event);
}



int MatrixModel::GetNumStrands() const {
    if (SingleChannel) {
        return 1;
    }
    return parm1*parm3;
}

void MatrixModel::InitModel() {
    if (DisplayAs == "Vert Matrix") {
        InitVMatrix();
        CopyBufCoord2ScreenCoord();
    } else if (DisplayAs == "Horiz Matrix") {
        InitHMatrix();
        CopyBufCoord2ScreenCoord();
    }
    DisplayAs = "Matrix";
}

// initialize buffer coordinates
// parm1=NumStrings
// parm2=PixelsPerString
// parm3=StrandsPerString
void MatrixModel::InitVMatrix(int firstExportStrand) {
    vMatrix = true;
    int y,x,idx,stringnum,segmentnum,yincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(PixelsPerStrand,NumStrands);
    SetNodeCount(parm1,PixelsPerString, rgbOrder);
    screenLocation.SetRenderSize(NumStrands, PixelsPerStrand);

    // create output mapping
    if (SingleNode) {
        x=0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            y=0;
            yincr=1;
            for (size_t c=0; c<PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX=IsLtoR ? x : NumStrands-x-1;
                Nodes[n]->Coords[c].bufY=y;
                y+=yincr;
                if (y < 0 || y >= PixelsPerStrand) {
                    yincr=-yincr;
                    y+=yincr;
                    x++;
                }
            }
        }
    } else {
        std::vector<int> strandStartChan;
        strandStartChan.clear();
        strandStartChan.resize(NumStrands);
        for (int x = 0; x < NumStrands; x++) {
            stringnum = x / parm3;
            segmentnum = x % parm3;
            strandStartChan[x] = stringStartChan[stringnum] + segmentnum * PixelsPerStrand * 3;
        }
        if (firstExportStrand > 0 && firstExportStrand < NumStrands) {
            int offset = strandStartChan[firstExportStrand];
            for (int x = 0; x < NumStrands; x++) {
                strandStartChan[x] = strandStartChan[x] - offset;
                if (strandStartChan[x] < 0) {
                    strandStartChan[x] += (PixelsPerStrand * NumStrands * 3);
                }
            }
        }
        
        for (x=0; x < NumStrands; x++) {
            stringnum = x / parm3;
            segmentnum = x % parm3;
            for(y=0; y < PixelsPerStrand; y++) {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + y;
                Nodes[idx]->ActChan = strandStartChan[x] + y*3;
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
void MatrixModel::InitHMatrix() {
    vMatrix = false;
    int y,x,idx,stringnum,segmentnum,xincr;
    if (parm3 > parm2) {
        parm3 = parm2;
    }
    int NumStrands=parm1*parm3;
    int PixelsPerStrand=parm2/parm3;
    int PixelsPerString=PixelsPerStrand*parm3;
    SetBufferSize(NumStrands,PixelsPerStrand);
    SetNodeCount(parm1,PixelsPerString,rgbOrder);
    screenLocation.SetRenderSize(PixelsPerStrand, NumStrands);
    
    // create output mapping
    if (SingleNode) {
        y=0;
        for (size_t n=0; n<Nodes.size(); n++) {
            Nodes[n]->ActChan = stringStartChan[n];
            x=0;
            xincr=1;
            for (size_t c=0; c<PixelsPerString; c++) {
                Nodes[n]->Coords[c].bufX=x;
                Nodes[n]->Coords[c].bufY=isBotToTop ? y :NumStrands-y-1;
                x+=xincr;
                if (x < 0 || x >= PixelsPerStrand) {
                    xincr=-xincr;
                    x+=xincr;
                    y++;
                }
            }
        }
    } else {
        for (y=0; y < NumStrands; y++) {
            stringnum=y / parm3;
            segmentnum=y % parm3;
            for(x=0; x<PixelsPerStrand; x++) {
                idx=stringnum * PixelsPerString + segmentnum * PixelsPerStrand + x;
                Nodes[idx]->ActChan = stringStartChan[stringnum] + segmentnum * PixelsPerStrand*3 + x*3;
                Nodes[idx]->Coords[0].bufX=IsLtoR != (segmentnum % 2 == 0) ? PixelsPerStrand-x-1 : x;
                Nodes[idx]->Coords[0].bufY= isBotToTop ? y :NumStrands-y-1;
                Nodes[idx]->StringNum=stringnum;
            }
        }
    }
}

