
#include <wx/tokenzr.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "TreeModel.h"
#include "ModelScreenLocation.h"


TreeModel::TreeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : MatrixModel(manager)
{
    treeType = 0;
    degrees = 360;
    SetFromXml(node, zeroBased);
}

TreeModel::~TreeModel()
{
}

void TreeModel::InitModel() {
    wxStringTokenizer tkz(DisplayAs, " ");
    wxString token = tkz.GetNextToken();
    
    int firstStrand = 0;
    if (zeroBased && ModelXml->GetAttribute("exportFirstStrand") != "") {
        firstStrand = wxAtoi(ModelXml->GetAttribute("exportFirstStrand")) - 1;
    }
    if (firstStrand < 0) {
        firstStrand = 0;
    }
    InitVMatrix(firstStrand);
    token = tkz.GetNextToken();
    token.ToLong(&degrees);
    treeType = 0;
    if (token == "Flat") {
        treeType = 1;
        degrees = 0;
    } else if (token == "Ribbon") {
        treeType = 2;
        degrees = -1;
    }
    rotation = wxAtof(ModelXml->GetAttribute("TreeRotation", "3"));
    spiralRotations = wxAtof(ModelXml->GetAttribute("TreeSpiralRotations", "0.0"));
    botTopRatio = wxAtof(ModelXml->GetAttribute("TreeBottomTopRatio", "6.0"));
    perspective = wxAtof(ModelXml->GetAttribute("TreePerspective", "0.2"));
    SetTreeCoord(degrees);
    DisplayAs = "Tree";
}

static inline double toRadians(float degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}
// initialize screen coordinates for tree
void TreeModel::SetTreeCoord(long degrees) {
    double bufferX, bufferY;
    if (BufferWi < 1) return;
    if (BufferHt < 1) return; // June 27,2013. added check to not divide by zero
    double RenderHt, RenderWi;
    if (degrees > 0) {
        double angle;
        RenderHt=BufferHt * 3;
        RenderWi=((double)RenderHt)/1.8;
        
        double radians=toRadians(degrees);
        double radius=RenderWi/2.0;
        double topradius=radius/botTopRatio;
        
        double StartAngle=-radians/2.0;
        double AngleIncr=radians/double(BufferWi);
        
        if (degrees > 180) {
            //shift a tiny bit to make the strands in back not line up exactly with the strands in front
            StartAngle += toRadians(rotation);
        }
        
        std::vector<float> yPos(BufferHt);
        std::vector<float> xInc(BufferHt);
        for (int x = 0; x < BufferHt; x ++) {
            yPos[x] = x;
            xInc[x] = 0;
        }
        if (spiralRotations != 0.0f) {
            std::vector<float> lengths(10);
            float rgap = (radius - topradius)/ 10.0;
            float total = 0;
            for (int x = 0; x < 10; x++) {
                lengths[x] = 2.0*M_PI*(radius - rgap*x) - rgap/2.0;
                lengths[x] *= spiralRotations / 10.0;
                lengths[x] = sqrt(lengths[x]*lengths[x]+(float)BufferHt/10.0*(float)BufferHt/10.0);
                total += lengths[x];
            }
            int lights = 0;
            for (int x = 0; x < 10; x++) {
                lengths[x] /= total;
                lights += (int)std::round(lengths[x]*BufferHt);
            }
            int curSeg = 0;
            float lightsInSeg = std::round(lengths[0]*BufferHt);
            int curLightInSeg = 0;
            for (int x = 1; x < BufferHt; x++) {
                if (curLightInSeg >= lightsInSeg) {
                    curSeg++;
                    curLightInSeg = 0;
                    lightsInSeg = std::round(lengths[curSeg]*BufferHt);
                }
                float ang = spiralRotations * 2.0 * M_PI / 10.0;
                ang /= (float)lightsInSeg;
                yPos[x] = yPos[x-1] + (BufferHt/10.0/lightsInSeg);
                xInc[x] = xInc[x-1] + ang;
                curLightInSeg++;
            }
        }
        
        
        double topYoffset = std::abs(perspective * topradius * cos(M_PI));
        double ytop = RenderHt - topYoffset;
        double ybot = std::abs(perspective * radius * cos(M_PI));
        
        size_t NodeCount=GetNodeCount();
        for(size_t n=0; n<NodeCount; n++) {
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++) {
                bufferX=Nodes[n]->Coords[c].bufX;
                bufferY=Nodes[n]->Coords[c].bufY;
                angle=StartAngle + double(bufferX) * AngleIncr + xInc[bufferY];
                double xb=radius * sin(angle);
                double xt=topradius * sin(angle);
                double yb = ybot - perspective * radius * cos(angle);
                double yt = ytop - perspective * topradius * cos(angle);
                double posOnString = 0.5;
                if (BufferHt > 1) {
                    posOnString = yPos[bufferY]/(double)(BufferHt-1.0);
                }

                Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                Nodes[n]->Coords[c].screenY = yb + (yt - yb) * posOnString - ((double)RenderHt)/2.0;
            }
        }
    } else {
        double treeScale = degrees == -1 ? 5.0 : 4.0;
        double botWid = BufferWi * treeScale;
        RenderHt=BufferHt * 2.0;
        RenderWi=(botWid + 2);
        
        double offset = 0.5;
        size_t NodeCount=GetNodeCount();
        for(size_t n=0; n<NodeCount; n++) {
            size_t CoordCount=GetCoordCount(n);
            if (degrees == -1) {
                for(size_t c=0; c < CoordCount; c++) {
                    bufferX=Nodes[n]->Coords[c].bufX;
                    bufferY=Nodes[n]->Coords[c].bufY;
                    
                    double xt = (bufferX + offset - BufferWi/2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi/2.0) * treeScale;
                    double h = std::sqrt(RenderHt * RenderHt + (xt - xb)*(xt - xb));
                    
                    double posOnString = 0.5;
                    if (BufferHt > 1) {
                        posOnString = (bufferY/(double)(BufferHt-1.0));
                    }
                    
                    double newh = RenderHt * posOnString;
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;
                    
                    posOnString = 0;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY - 0.33)/(double)(BufferHt-1.0));
                    }

                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;
                    
                    posOnString = 1;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY + 0.33)/(double)(BufferHt-1.0));
                    }
                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;
                }
                
            } else {
                for(size_t c=0; c < CoordCount; c++) {
                    bufferX=Nodes[n]->Coords[c].bufX;
                    bufferY=Nodes[n]->Coords[c].bufY;
                    
                    double xt = (bufferX + offset - BufferWi/2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi/2.0) * treeScale;
                    double posOnString = 0.5;
                    if (BufferHt > 1) {
                        posOnString = (bufferY/(double)(BufferHt-1.0));
                    }
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * posOnString - ((double)RenderHt)/2.0;
                }
            }
        }
    }
    screenLocation.SetRenderSize(RenderWi, RenderHt);
}
int TreeModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "TreeStyle") {
        ModelXml->DeleteAttribute("DisplayAs");
        wxPGProperty *p = grid->GetPropertyByName("TreeDegrees");
        if (p != nullptr) {
            degrees = p->GetValue().GetLong();
        }
        switch (event.GetPropertyValue().GetLong()) {
            case 0:
                ModelXml->AddAttribute("DisplayAs", wxString::Format("Tree %d", degrees > 1 ? degrees : 180));
                break;
            case 1:
                ModelXml->AddAttribute("DisplayAs", "Tree Flat");
                break;
            case 2:
                ModelXml->AddAttribute("DisplayAs", "Tree Ribbon");
                break;
        }
        SetFromXml(ModelXml, zeroBased);
        if (p != nullptr) {
            p->Enable(treeType == 0);
        }
        p = grid->GetPropertyByName("TreeRotation");
        if (p != nullptr) {
            p->Enable(treeType == 0);
        }
        return 3;
    } else if (event.GetPropertyName() == "TreeDegrees") {
        ModelXml->DeleteAttribute("DisplayAs");
        ModelXml->AddAttribute("DisplayAs", wxString::Format("Tree %d", event.GetPropertyValue().GetLong()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if (event.GetPropertyName() == "TreeRotation") {
        ModelXml->DeleteAttribute("TreeRotation");
        ModelXml->AddAttribute("TreeRotation", wxString::Format("%f", (float)event.GetPropertyValue().GetDouble()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if (event.GetPropertyName() == "TreeSpiralRotations") {
        ModelXml->DeleteAttribute("TreeSpiralRotations");
        ModelXml->AddAttribute("TreeSpiralRotations", wxString::Format("%f", (float)event.GetPropertyValue().GetDouble()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if (event.GetPropertyName() == "TreeBottomTopRatio") {
        ModelXml->DeleteAttribute("TreeBottomTopRatio");
        ModelXml->AddAttribute("TreeBottomTopRatio", wxString::Format("%f", (float)event.GetPropertyValue().GetDouble()));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    } else if (event.GetPropertyName() == "TreePerspective") {
        ModelXml->DeleteAttribute("TreePerspective");
        ModelXml->AddAttribute("TreePerspective", wxString::Format("%f", (float)(event.GetPropertyValue().GetDouble()/10.0)));
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }
    return MatrixModel::OnPropertyGridChange(grid, event);
}
static wxPGChoices TREE_STYLES;

void TreeModel::AddStyleProperties(wxPropertyGridInterface *grid) {
    if (TREE_STYLES.GetCount() == 0) {
        TREE_STYLES.Add("Round");
        TREE_STYLES.Add("Flat");
        TREE_STYLES.Add("Ribbon");
    }
    grid->Append(new wxEnumProperty("Type", "TreeStyle", TREE_STYLES, treeType));
    
    wxPGProperty *p = grid->Append(new wxUIntProperty("Degrees", "TreeDegrees", treeType == 0 ? degrees : 180));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "360");
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);
    
    p = grid->Append(new wxFloatProperty("Rotation", "TreeRotation", treeType == 0 ? rotation : 3));
    p->SetAttribute("Min", "-360");
    p->SetAttribute("Max", "360");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);
    
    p = grid->Append(new wxFloatProperty("Spiral Wraps", "TreeSpiralRotations", treeType == 0 ? spiralRotations : 0.0));
    p->SetAttribute("Min", "-20");
    p->SetAttribute("Max", "20");
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);
    
    p = grid->Append(new wxFloatProperty("Bottom/Top Ratio", "TreeBottomTopRatio", treeType == 0 ? botTopRatio : 6.0));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "50");
    p->SetAttribute("Step", 0.5);
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);
    p = grid->Append(new wxFloatProperty("Perspective", "TreePerspective", treeType == 0 ? perspective*10 : 2));
    p->SetAttribute("Min", "0");
    p->SetAttribute("Max", "10");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);
}

