
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "ModelGroup.h"
#include "ModelManager.h"
#include "SingleLineModel.h"


static const std::string HORIZ_PER_MODEL("Horizontal Per Model");
static const std::string VERT_PER_MODEL("Vertical Per Model");
static const std::string HORIZ_PER_MODELSTRAND("Horizontal Per Model/Strand");
static const std::string VERT_PER_MODELSTRAND("Vertical Per Model/Strand");
static const std::string OVERLAY_CENTER("Overlay - Centered");
static const std::string OVERLAY_SCALED("Overlay - Scaled");
static const std::string SINGLE_LINE("Single Line");


std::vector<std::string> ModelGroup::GROUP_BUFFER_STYLES;
const std::vector<std::string> &ModelGroup::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            GROUP_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            GROUP_BUFFER_STYLES.push_back(HORIZ_PER_MODEL);
            GROUP_BUFFER_STYLES.push_back(VERT_PER_MODEL);
            GROUP_BUFFER_STYLES.push_back(HORIZ_PER_MODELSTRAND);
            GROUP_BUFFER_STYLES.push_back(VERT_PER_MODELSTRAND);
            GROUP_BUFFER_STYLES.push_back(OVERLAY_CENTER);
            GROUP_BUFFER_STYLES.push_back(OVERLAY_SCALED);
        }
    };
    static Initializer ListInitializationGuard;
    return GROUP_BUFFER_STYLES;
}



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
        defaultBufferStyle = VERT_PER_MODEL;
    } else if (layout == "horizontal") {
        defaultBufferStyle = HORIZ_PER_MODEL;
    }

    
    std::vector<Model*> models;
    wxArrayString mn = wxSplit(node->GetAttribute("models"), ',');
    for (int x = 0; x < mn.size(); x++) {
        Model *c = manager.GetModel(mn[x].ToStdString());
        if (c != nullptr) {
            modelNames.push_back(c->name);
            models.push_back(c);
            
            
            int bw, bh;
            c->InitRenderBufferNodes("Per Preview No Offset", "None", Nodes, bw, bh);
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
    if (miny < 0) {
        for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
            for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); coord++) {
                coord->screenY -= miny;
            }
        }
        maxy -= miny;
        miny = 0;
    }
    if (minx < 0) {
        for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
            for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); coord++) {
                coord->screenX -= minx;
            }
        }
        maxx -= minx;
        minx = 0;
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


void ModelGroup::GetBufferSize(const std::string &tp, const std::string &transform, int &BufferWi, int &BufferHi) const {
    std::string type = tp;
    if (type == "Default") {
        type = defaultBufferStyle;
    }
    int models = 0;
    int strands = 0;
    int maxStrandLen = 0;
    int maxNodes = 0;
    int total = 0;
    
    int maxWid = 0;
    int maxHi = 0;
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
            
            maxWid = std::max(maxWid, m->GetDefaultBufferWi());
            maxHi = std::max(maxHi, m->GetDefaultBufferHt());
        }
    }
    if (type == HORIZ_PER_MODEL) {
        BufferWi = models;
        BufferHi = maxNodes;
    } else if (type == VERT_PER_MODEL) {
        BufferHi = models;
        BufferWi = maxNodes;
    } else if (type == HORIZ_PER_MODELSTRAND) {
        BufferWi = strands;
        BufferHi = maxStrandLen;
    } else if (type == VERT_PER_MODELSTRAND) {
        BufferHi = strands;
        BufferWi = maxStrandLen;
    } else if (type == SINGLE_LINE) {
        BufferHi = 1;
        BufferWi = total;
    } else if (type == OVERLAY_CENTER || type == OVERLAY_SCALED) {
        BufferHi = maxHi;
        BufferWi = maxWid;
    } else {
        Model::GetBufferSize(type, "None", BufferWi, BufferHi);
    }
    AdjustForTransform(transform, BufferWi, BufferHi);
}
void ModelGroup::InitRenderBufferNodes(const std::string &tp,
                                       const std::string &transform,
                                       std::vector<NodeBaseClassPtr> &Nodes,
                                       int &BufferWi, int &BufferHi) const {
    std::string type = tp;
    if (type == "Default") {
        type = defaultBufferStyle;
    }
    BufferWi = BufferHi = 0;

    if (type == HORIZ_PER_MODEL) {
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "None", Nodes, x, y);
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
        ApplyTransform(transform, Nodes, BufferWi, BufferHi);
    } else if (type == VERT_PER_MODEL) {
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "None", Nodes, x, y);
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
        ApplyTransform(transform, Nodes, BufferWi, BufferHi);
    } else if (type == HORIZ_PER_MODELSTRAND || type == VERT_PER_MODELSTRAND) {
        GetBufferSize(type, "None", BufferWi, BufferHi);
        bool horiz = type == HORIZ_PER_MODELSTRAND;
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
                    slm.Reset(slen, *m, strand, -1, true);
                    int start = Nodes.size();
                    int x, y;
                    slm.InitRenderBufferNodes("Default", "None", Nodes, x, y);
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
        ApplyTransform(transform, Nodes, BufferWi, BufferHi);
    } else if (type == SINGLE_LINE) {
        BufferHi = 1;
        BufferWi = 0;
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Single Line", "None", Nodes, x, y);
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
        ApplyTransform(transform, Nodes, BufferWi, BufferHi);
    } else if (type == OVERLAY_CENTER || type == OVERLAY_SCALED) {
        bool scale = type == OVERLAY_SCALED;
        GetBufferSize(type, "None", BufferWi, BufferHi);
        for (auto it = modelNames.begin(); it != modelNames.end(); it++) {
            Model* m = manager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int bw, bh;
                m->InitRenderBufferNodes("Default", "None", Nodes, bw, bh);
                if (bw != BufferWi || bh != BufferHi) {
                    //need to either scale or center
                    int offx = (BufferWi - bw)/2;
                    int offy = (BufferHi - bh)/2;
                    while (start < Nodes.size()) {
                        for (auto it = Nodes[start]->Coords.begin(); it != Nodes[start]->Coords.end(); it++) {
                            if (scale) {
                                it->bufX = it->bufX * (BufferWi/bw);
                                it->bufY = it->bufY * (BufferHi/bh);
                            } else {
                                it->bufX += offx;
                                it->bufY += offy;
                            }
                        }
                        start++;
                    }
                }
            }
        }
    } else {
        Model::InitRenderBufferNodes(type, transform, Nodes, BufferWi, BufferHi);
    }
}
