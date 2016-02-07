
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "CustomModel.h"

CustomModel::CustomModel(wxXmlNode *node, const NetInfoClass &netInfo, bool zeroBased)
{
    SetFromXml(node, netInfo, zeroBased);
    
}

CustomModel::~CustomModel()
{
}



int CustomModel::GetStrandLength(int strand) const {
    return Nodes.size();
}

int CustomModel::MapToNodeIndex(int strand, int node) const {
    return node;
}


void CustomModel::InitModel() {
    std::string customModel = ModelXml->GetAttribute("CustomModel").ToStdString();
    InitCustomMatrix(customModel);
    CopyBufCoord2ScreenCoord();

}

void CustomModel::SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    std::string customModel = ModelXml->GetAttribute("CustomModel").ToStdString();
    int maxval=GetCustomMaxChannel(customModel);
    // fix NumberOfStrings
    if (SingleNode) {
        NumberOfStrings=maxval;
    } else {
        ChannelsPerString=maxval*GetNodeChannelCount(StringType);
    }
    Model::SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
}



int CustomModel::GetCustomMaxChannel(const std::string& customModel) {
    wxString value;
    wxArrayString cols;
    long val,maxval=0;
    wxString valstr;
    
    wxArrayString rows=wxSplit(customModel,';');
    for(size_t row=0; row < rows.size(); row++) {
        cols=wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++) {
            valstr=cols[col];
            if (!valstr.IsEmpty() && valstr != "0") {
                valstr.ToLong(&val);
                maxval=std::max(val,maxval);
            }
        }
    }
    return maxval;
}
void CustomModel::InitCustomMatrix(const std::string& customModel) {
    wxString value;
    wxArrayString cols;
    long idx;
    int width=1;
    std::vector<int> nodemap;
    
    wxArrayString rows=wxSplit(customModel,';');
    int height=rows.size();
    int cpn = -1;
   
    for(size_t row=0; row < rows.size(); row++) {
        cols=wxSplit(rows[row],',');
        if (cols.size() > width) width=cols.size();
        for(size_t col=0; col < cols.size(); col++) {
            value=cols[col];
            if (!value.IsEmpty() && value != "0") {
                value.ToLong(&idx);
                
                // increase nodemap size if necessary
                if (idx > nodemap.size()) {
                    nodemap.resize(idx, -1);
                }
                idx--;  // adjust to 0-based
                
                // is node already defined in map?
                if (nodemap[idx] < 0) {
                    // unmapped - so add a node
                    nodemap[idx]=Nodes.size();
                    SetNodeCount(1,0,rgbOrder);  // this creates a node of the correct class
                    Nodes.back()->StringNum=idx;
                    if (cpn == -1) {
                        cpn = GetChanCountPerNode();
                    }
                    Nodes.back()->ActChan=stringStartChan[0] + idx * cpn;
                    if (idx < nodeNames.size()) {
                        Nodes.back()->SetName(nodeNames[idx]);
                    }
                    Nodes.back()->AddBufCoord(col,height - row - 1);
                } else {
                    // mapped - so add a coord to existing node
                    Nodes[nodemap[idx]]->AddBufCoord(col,height - row - 1);
                }
            }
        }
    }
    for (int x = 0; x < Nodes.size(); x++) {
        for (int y = x+1; y < Nodes.size(); y++) {
            if (Nodes[y]->StringNum < Nodes[x]->StringNum) {
                Nodes[x].swap(Nodes[y]);
            }
        }
    }
    for (int x = 0; x < Nodes.size(); x++) {
        Nodes[x]->SetName(GetNodeName(Nodes[x]->StringNum));
    }
    
    SetBufferSize(height,width);
}
