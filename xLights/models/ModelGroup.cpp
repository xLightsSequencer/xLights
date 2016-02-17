
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "ModelGroup.h"
#include "ModelManager.h"
#include "SingleLineModel.h"


std::vector<std::string> ModelGroup::GROUP_BUFFER_STYLES;

ModelGroup::ModelGroup(wxXmlNode *node, NetInfoClass &netInfo, ModelManager &m, int previewW, int previewH)
    : Model(), manager(m)
{
    selected = node->GetAttribute("selected", "0") == "1";
    name = node->GetAttribute("name").ToStdString();
    DisplayAs = "ModelGroup";
    StringType = "RGB Nodes";
    ModelNetInfo = &netInfo;
    ModelXml = node;
    
    int gridSize = wxAtoi(node->GetAttribute("GridSize", "400"));
    std::string layout = node->GetAttribute("layout", "grid").ToStdString();
    defaultBufferStyle = layout;
    if (layout == "grid" || layout == "minimalGrid") {
        defaultBufferStyle = "Default";
    } else if (layout == "vertical") {
        defaultBufferStyle = "Vertical Per Model";
    } else if (layout == "horizontal") {
        defaultBufferStyle = "Horizontal Per Model";
    }

    
    std::vector<Model*> models;
    wxArrayString mn = wxSplit(node->GetAttribute("models"), ',');
    for (int x = 0; x < mn.size(); x++) {
        Model *c = manager.GetModel(mn[x].ToStdString());
        if (c != nullptr) {
            modelNames.push_back(c->name);
            models.push_back(c);
            
            
            int bw, bh;
            c->InitRenderBufferNodes("Per Preview No Offset", Nodes, bw, bh);
        }
    }
    
    //now have all the nodes for all the models
    float minx = 99999;
    float maxx = -1;
    float miny = 99999;
    float maxy = -1;
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); coord++) {
            minx = std::min(minx, coord->screenX);
            miny = std::min(miny, coord->screenY);
            maxx = std::max(maxx, coord->screenX);
            maxy = std::max(maxy, coord->screenY);
        }
    }
    float midX = (minx + maxx) / 2.0;
    float midY = (miny + maxy) / 2.0;
    bool minimal = layout != "grid";
    
    double hscale = gridSize / maxy;
    double wscale = gridSize / maxx;
    if (maxy < gridSize && maxx < gridSize) {
        hscale = 1.0;
        wscale = 1.0;
    }
    if (minimal) {
        if ((maxy-miny+1) < gridSize && (maxx-minx+1) < gridSize) {
            hscale = 1.0;
            wscale = 1.0;
        } else {
            hscale = gridSize / (maxy - miny + 1);
            wscale = gridSize / (maxx - minx + 1);
        }
    }
    if (hscale > wscale) {
        hscale = wscale;
    } else {
        wscale = hscale;
    }
    maxx = -999999;
    maxy = -999999;
    float nminx = 999999;
    float nminy = 999999;
    BufferHt = 0;
    BufferWi = 0;
    
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); coord++) {
            if (minimal) {
                coord->screenX = coord->screenX - minx;
                coord->screenY = coord->screenY - miny;
            }
            coord->bufX = coord->screenX * wscale;
            coord->bufY = coord->screenY * hscale;
            if (minimal) {
                coord->screenX = coord->screenX + minx - midX;
                coord->screenY = coord->screenY + miny - midY;
            } else {
                coord->screenX = coord->screenX - midX;
                coord->screenY = coord->screenY - midY;
            }
            maxx = std::max(maxx, (float)coord->screenX);
            maxy = std::max(maxy, (float)coord->screenY);
            nminx = std::min(nminx, (float)coord->screenX);
            nminy = std::min(nminy, (float)coord->screenY);
            BufferHt = std::max(BufferHt, (int)coord->bufY);
            BufferWi = std::max(BufferWi, (int)coord->bufX);
        }
    }
    
    BufferHt++;
    BufferWi++;
    RenderWi = maxx - nminx + 1;
    RenderHt = maxy - nminy + 1;
    
    offsetXpct = offsetYpct = 0.5;
    
    SetMinMaxModelScreenCoordinates(previewW, previewH);
}

ModelGroup::~ModelGroup()
{
}
const std::vector<std::string> &ModelGroup::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            GROUP_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            GROUP_BUFFER_STYLES.push_back("Horizontal Per Model");
            GROUP_BUFFER_STYLES.push_back("Vertical Per Model");
            GROUP_BUFFER_STYLES.push_back("Horizontal Per Model/Strand");
            GROUP_BUFFER_STYLES.push_back("Vertical Per Model/Strand");
        }
    };
    static Initializer ListInitializationGuard;
    return GROUP_BUFFER_STYLES;
}


void ModelGroup::GetBufferSize(const std::string &tp, int &BufferWi, int &BufferHi) const {
    std::string type = tp;
    if (type == "Default") {
        type = defaultBufferStyle;
    }
    int models = 0;
    int strands = 0;
    int maxStrandLen = 0;
    int maxNodes = 0;
    int total = 0;
    for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
        Model* m = manager[*it];
        if (m != nullptr) {
            models++;
            strands += m->GetNumStrands();
            total += m->GetNodeCount();
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
    } else if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = total;
    } else {
        Model::GetBufferSize(type, BufferWi, BufferHi);
    }
}
void ModelGroup::InitRenderBufferNodes(const std::string &tp, std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const {
    std::string type = tp;
    if (type == "Default") {
        type = defaultBufferStyle;
    }
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
    } else if (type == "Single Line") {
        BufferHi = 1;
        BufferWi = 0;
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Single Line", Nodes, x, y);
                while (start < Nodes.size()) {
                    for (auto it = Nodes[start]->Coords.begin(); it != Nodes[start]->Coords.end(); it++) {
                        it->bufX = BufferWi;
                        it->bufY = 0;
                    }
                    start++;
                    BufferWi++;
                }
            }
        }
    } else {
        Model::InitRenderBufferNodes(type, Nodes, BufferWi, BufferHi);
    }
}
