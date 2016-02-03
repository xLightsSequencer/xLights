
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "ModelGroup.h"
#include "ModelManager.h"

ModelGroup::ModelGroup(wxXmlNode *node, NetInfoClass &netInfo, ModelManager &manager, int previewW, int previewH)
{
    wxXmlNode *e = CreateModelNodeFromGroup(manager, previewW, previewH, node);
    SetFromXml(e, netInfo);
    selected = node->GetAttribute("selected", "0") == "1";
}

ModelGroup::~ModelGroup()
{
}




wxXmlNode* ModelGroup::CreateModelNodeFromGroup(ModelManager &manager, int previewW, int previewH, wxXmlNode *e) {
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
    wxXmlNode * ret = BuildWholeHouseModel(manager, previewW, previewH, name, e, models);
    ret->AddAttribute("models", e->GetAttribute("models"));
    return ret;
}
wxXmlNode *ModelGroup::BuildWholeHouseModel(ModelManager &manager, int previewW, int previewH,
                                            const std::string &modelName, const wxXmlNode *node, std::vector<Model*> &models)
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
            models[i]->AddToWholeHouseModel(previewW, previewH, xPos,yPos,actChannel,nodeType);
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
        
        for(int i=0;i<xPos.size();i++) {
            e->AddAttribute("parm2", wxString::Format(wxT("%i"), hor ? max : models.size()));
            e->AddAttribute("parm1", wxString::Format(wxT("%i"), hor ? models.size() : max));
            
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