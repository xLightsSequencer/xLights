
#include <wx/xml/xml.h>
#include <wx/tokenzr.h>

#include "TreeModel.h"

TreeModel::TreeModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased)
{
    SetFromXml(node, netInfo, zeroBased);
}

TreeModel::~TreeModel()
{
}

int TreeModel::GetNumStrands() const {
    return parm1*parm3;
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
    long degrees;
    token = tkz.GetNextToken();
    token.ToLong(&degrees);
    if (token == "Flat") {
        degrees = 0;
    } else if (token == "Ribbon") {
        degrees = -1;
    }
    SetTreeCoord(degrees);
}

inline double toRadians(long degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}
// initialize screen coordinates for tree
void TreeModel::SetTreeCoord(long degrees) {
    double bufferX, bufferY;
    if (BufferWi < 2) return;
    if (BufferHt < 1) return; // June 27,2013. added check to not divide by zero
    if (degrees > 0) {
        double angle;
        RenderHt=1000;
        RenderWi=((double)RenderHt)/1.8;
        
        double radians=toRadians(degrees);
        double radius=RenderWi/2.0;
        double topradius=RenderWi/12;
        
        double StartAngle=-radians/2.0;
        double AngleIncr=radians/double(BufferWi);
        if (degrees > 180) {
            //shift a tiny bit to make the strands in back not line up exactly with the strands in front
            StartAngle += AngleIncr / 5.0;
        }
        double topYoffset = std::abs(0.2 * topradius * cos(M_PI));
        double ytop = RenderHt - topYoffset;
        double ybot = std::abs(0.2 * radius * cos(M_PI));
        
        size_t NodeCount=GetNodeCount();
        for(size_t n=0; n<NodeCount; n++) {
            size_t CoordCount=GetCoordCount(n);
            for(size_t c=0; c < CoordCount; c++) {
                bufferX=Nodes[n]->Coords[c].bufX;
                bufferY=Nodes[n]->Coords[c].bufY;
                angle=StartAngle + double(bufferX) * AngleIncr;
                double xb=radius * sin(angle);
                double xt=topradius * sin(angle);
                double yb = ybot - 0.2 * radius * cos(angle);
                double yt = ytop - 0.2 * topradius * cos(angle);
                double posOnString = (bufferY/(double)(BufferHt-1.0));
                
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
                    
                    double posOnString = (bufferY/(double)(BufferHt-1.0));
                    double newh = RenderHt * posOnString;
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;
                    
                    posOnString = ((bufferY - 0.33)/(double)(BufferHt-1.0));
                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt)/2.0;
                    
                    posOnString = ((bufferY + 0.33)/(double)(BufferHt-1.0));
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
                    double posOnString = (bufferY/(double)(BufferHt-1.0));
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * posOnString - ((double)RenderHt)/2.0;
                }
            }
        }
    }
}