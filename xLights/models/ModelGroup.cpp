
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "ModelGroup.h"
#include "ModelManager.h"
#include "SingleLineModel.h"


std::vector<std::string> ModelGroup::GROUP_BUFFER_STYLES {
        "Default", "Per Preview", "Rotate CC 90",
        "Rotate CW 90", "Rotate 180", "Flip Vertical", "Flip Horizontal",
        "Horizontal Per Model", "Vertical Per Model",
        "Horizontal Per Model/Strand", "Vertical Per Model/Strand",
        };

ModelGroup::ModelGroup(wxXmlNode *node, NetInfoClass &netInfo, ModelManager &m, int previewW, int previewH)
    : WholeHouseModel(), manager(m)
{
    wxXmlNode *e = CreateModelNodeFromGroup(previewW, previewH, node);
    SetFromXml(e, netInfo);
    selected = node->GetAttribute("selected", "0") == "1";
    DisplayAs = "ModelGroup";
}

ModelGroup::~ModelGroup()
{
}


void ModelGroup::GetBufferSize(const std::string &type, int &BufferWi, int &BufferHi) const {
    int models = 0;
    int strands = 0;
    int maxStrandLen = 0;
    int maxNodes = 0;
    for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
        Model* m = manager[*it];
        if (m != nullptr) {
            models++;
            strands += m->GetNumStrands();
            if (m->GetNodeCount() > maxNodes) {
                maxNodes = m->GetNodeCount();
            }
            for (int x = 0; x < m->GetNumStrands(); x++) {
                if (m->GetStrandLength(x) > maxStrandLen) {
                    maxStrandLen = m->GetStrandLength(x);
                }
            }
        }
    }
    if (type == "Horizontal Per Model") {
        BufferWi = models;
        BufferHi = maxNodes;
    } else if (type == "Vertical Per Model") {
        BufferHi = models;
        BufferWi = maxNodes;
    } else if (type == "Horizontal Per Model/Strand") {
        BufferWi = strands;
        BufferHi = maxStrandLen;
    } else if (type == "Vertical Per Model/Strand") {
        BufferHi = strands;
        BufferWi = maxStrandLen;
    } else {
        Model::GetBufferSize(type, BufferWi, BufferHi);
    }
}
void ModelGroup::InitRenderBufferNodes(const std::string &type, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const {
    BufferWi = BufferHi = 0;
    
    if (type == "Horizontal Per Model") {
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", Nodes, x, y);
                y = 0;
                while (start < Nodes.size()) {
                    for (auto it = Nodes[start]->Coords.begin(); it != Nodes[start]->Coords.end(); it++) {
                        it->bufX = BufferWi;
                        it->bufY = y;
                        y++;
                    }
                    start++;
                }
                if (y > BufferHi) {
                    BufferHi = y;
                }
                BufferWi++;
            }
        }
    } else if (type == "Vertical Per Model") {
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", Nodes, x, y);
                y = 0;
                while (start < Nodes.size()) {
                    for (auto it = Nodes[start]->Coords.begin(); it != Nodes[start]->Coords.end(); it++) {
                        it->bufY = BufferHi;
                        it->bufX = y;
                        y++;
                    }
                    start++;
                }
                if (y > BufferWi) {
                    BufferWi = y;
                }
                BufferHi++;
            }
        }
    } else if (type == "Horizontal Per Model/Strand" || type == "Vertical Per Model/Strand") {
        GetBufferSize(type, BufferWi, BufferHi);
        bool horiz = type == "Horizontal Per Model/Strand";
        int curS = 0;
        double maxSL = BufferWi;
        if (horiz) {
            maxSL = BufferHi;
        }
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                SingleLineModel slm;
                for (int strand = 0; strand < m->GetNumStrands(); strand++) {
                    int slen = m->GetStrandLength(strand);
                    slm.Reset(slen, *m, strand);
                    int start = Nodes.size();
                    int x, y;
                    slm.InitRenderBufferNodes("Default", Nodes, x, y);
                    y = 0;
                    while (start < Nodes.size()) {
                        for (auto it = Nodes[start]->Coords.begin(); it != Nodes[start]->Coords.end(); it++) {
                            int newY = ((double)y * maxSL / (double)slen);
                            if (horiz) {
                                it->bufX = curS;
                                it->bufY = newY;
                            } else {
                                it->bufX = newY;
                                it->bufY = curS;
                            }
                        }
                        y++;
                        start++;
                    }
                    curS++;
                }
            }
        }
    } else {
        Model::InitRenderBufferNodes(type, Nodes, BufferWi, BufferHi);
    }
}


wxXmlNode* ModelGroup::CreateModelNodeFromGroup(int previewW, int previewH, wxXmlNode *e) {
    std::vector<Model*> models;
    std::string name = e->GetAttribute("name").ToStdString();
    
    wxArrayString mn = wxSplit(e->GetAttribute("models"), ',');
    for (int x = 0; x < mn.size(); x++) {
        Model *c = manager.GetModel(mn[x].ToStdString());
        if (c != nullptr) {
            modelNames.push_back(c->name);
            models.push_back(c);
        }
    }
    wxXmlNode * ret = BuildWholeHouseModel(previewW, previewH, name, e, models);
    ret->AddAttribute("models", e->GetAttribute("models"));
    return ret;
}
wxXmlNode *ModelGroup::BuildWholeHouseModel(int previewW, int previewH,
                                            const std::string &modelName,
                                            const wxXmlNode *node, std::vector<Model*> &models)
{
    size_t numberOfNodes=0;
    int w,h;
    size_t index=0;
    wxString WholeHouseData="";
    
    for (int i=0; i<models.size(); i++)
    {
        numberOfNodes+= models[i]->GetNodeCount();
    }
    std::vector<int> xPos;
    std::vector<int> yPos;
    std::vector<int> actChannel;
    std::vector<std::string> nodeType;
    
    
    wxXmlNode* e=new wxXmlNode(wxXML_ELEMENT_NODE, "model");
    e->AddAttribute("name", modelName);
    e->AddAttribute("DisplayAs", "WholeHouse");
    e->AddAttribute("StringType", "RGB Nodes");
    
    wxString layout = node == nullptr ? "grid" : node->GetAttribute("layout", "grid");
    if (layout == "grid" || layout == "minimalGrid") {
        h = previewH;
        w = previewW;
        
        // Add node position and channel number to arrays
        for (int i=0; i<models.size(); i++)
        {
            models[i]->AddToWholeHouseModel(previewW, previewH, xPos, yPos, actChannel, nodeType);
            index+=models[i]->GetNodeCount();
        }
        int wScaled = node == nullptr ? 400 : wxAtoi(node->GetAttribute("GridSize", "400"));
        int hScaled = wScaled;
        int xOff = 0;
        int yOff = 0;
        if (layout == "minimalGrid") {
            int minx = 99999;
            int maxx = -1;
            int miny = 99999;
            int maxy = -1;
            for(int i=0;i<xPos.size();i++) {
                if (xPos[i] > maxx) {
                    maxx = xPos[i];
                }
                if (xPos[i] < minx) {
                    minx = xPos[i];
                }
                if (yPos[i] > maxy) {
                    maxy = yPos[i];
                }
                if (yPos[i] < miny) {
                    miny = yPos[i];
                }
            }
            xOff = minx;
            yOff = miny;
            
            h = maxy - miny + 1;
            w = maxx - minx + 1;
        }
        
        double hscale = (double)hScaled / (double)h;
        double wscale = (double)wScaled / (double)w;
        
        if (hscale > wscale) {
            hscale = wscale;
            hScaled = wscale * h + 1;
        } else {
            wscale = hscale;
            wScaled = hscale * w + 1;
        }
        // Create a new model node
        e->AddAttribute("parm1", wxString::Format(wxT("%i"), wScaled));
        e->AddAttribute("parm2", wxString::Format(wxT("%i"), hScaled));
        
        for(int i=0;i<xPos.size();i++)
        {
            xPos[i] = (int)(wscale*(double)(xPos[i] - xOff));
            yPos[i] = (int)((hscale*(double)(yPos[i] - yOff)));
            WholeHouseData += wxString::Format(wxT("%i,%i,%i,%s"),actChannel[i],xPos[i],yPos[i],(const char *)nodeType[i].c_str());
            if(i!=xPos.size()-1)
            {
                WholeHouseData+=";";
            }
        }
    } else {
        int max = 0;
        for (int i=0; i<models.size(); i++) {
            if (models[i] ->GetNodeCount() > max) {
                max = models[i]->GetNodeCount();
            }
        }
        for (int i=0; i<models.size(); i++) {
            for (int x = 0; x < models[i]->GetNodeCount(); x++) {
                xPos.push_back(x);
                yPos.push_back(i);
                actChannel.push_back(models[i]->NodeStartChannel(x));
                nodeType.push_back(models[i]->NodeType(x));
            }
        }
        bool hor = layout == "horizontal";

        e->AddAttribute("parm2", wxString::Format(wxT("%i"), hor ? max : models.size()));
        e->AddAttribute("parm1", wxString::Format(wxT("%i"), hor ? models.size() : max));
        
        for(int i=0;i<xPos.size();i++) {
            WholeHouseData += wxString::Format(wxT("%i,%i,%i,%s"),
                                               actChannel[i],
                                               hor ? yPos[i] : xPos[i],
                                               hor ? xPos[i] : yPos[i],
                                               (const char *)nodeType[i].c_str());
            if (i != xPos.size()-1) {
                WholeHouseData+=";";
            }
        }
    }
    
    e->AddAttribute("WholeHouseData", WholeHouseData);
    return e;
}