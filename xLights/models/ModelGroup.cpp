
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "ModelGroup.h"
#include "ModelManager.h"
#include "SingleLineModel.h"
#include "ModelScreenLocation.h"
#include <log4cpp/Category.hh>


static const std::string HORIZ_PER_MODEL("Horizontal Per Model");
static const std::string VERT_PER_MODEL("Vertical Per Model");
static const std::string HORIZ_PER_MODELSTRAND("Horizontal Per Model/Strand");
static const std::string VERT_PER_MODELSTRAND("Vertical Per Model/Strand");
static const std::string OVERLAY_CENTER("Overlay - Centered");
static const std::string OVERLAY_SCALED("Overlay - Scaled");
static const std::string SINGLE_LINE("Single Line");

static const std::string PER_MODEL_DEFAULT("Per Model Default");
static const std::string PER_MODEL_PER_PREVIEW("Per Model Per Preview");
static const std::string PER_MODEL_SINGLE_LINE("Per Model Single Line");


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

            GROUP_BUFFER_STYLES.push_back(PER_MODEL_DEFAULT);
            GROUP_BUFFER_STYLES.push_back(PER_MODEL_PER_PREVIEW);
            GROUP_BUFFER_STYLES.push_back(PER_MODEL_SINGLE_LINE);
        }
    };
    static Initializer ListInitializationGuard;
    return GROUP_BUFFER_STYLES;
}



ModelGroup::ModelGroup(wxXmlNode *node, const ModelManager &m, int w, int h) : ModelWithScreenLocation(m)
{
    ModelXml = node;
    screenLocation.previewW = w;
    screenLocation.previewH = h;
    Reset();
}

void LoadRenderBufferNodes(Model *m, const std::string &type, std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHi) {
    ModelGroup *g = dynamic_cast<ModelGroup*>(m);
    if (g != nullptr) {
        for (auto it = g->Models().begin(); it != g->Models().end(); it++) {
            LoadRenderBufferNodes(*it, type, newNodes, bufferWi, bufferHi);
        }
    } else {
        m->InitRenderBufferNodes(type, "None", newNodes, bufferWi, bufferHi);
    }
}

bool ModelGroup::Reset(bool zeroBased) {
    this->zeroBased = zeroBased;
    selected = false;
    name = ModelXml->GetAttribute("name").ToStdString();
    DisplayAs = "ModelGroup";
    StringType = "RGB Nodes";

    int gridSize = wxAtoi(ModelXml->GetAttribute("GridSize", "400"));
    std::string layout = ModelXml->GetAttribute("layout", "minimalGrid").ToStdString();
    defaultBufferStyle = layout;
    if (layout.compare(0, 9, "Per Model") == 0) {
        layout = "Default";
    }
    if (layout == "grid" || layout == "minimalGrid") {
        defaultBufferStyle = "Default";
    } else if (layout == "vertical") {
        defaultBufferStyle = VERT_PER_MODEL;
    } else if (layout == "horizontal") {
        defaultBufferStyle = HORIZ_PER_MODEL;
    }
    Nodes.clear();
    models.clear();
    modelNames.clear();
    changeCount = 0;
    wxArrayString mn = wxSplit(ModelXml->GetAttribute("models"), ',');
    for (int x = 0; x < mn.size(); x++) {
        Model *c = modelManager.GetModel(mn[x].ToStdString());
        if (c != nullptr) {
            modelNames.push_back(c->GetFullName());
            models.push_back(c);
            changeCount += c->GetChangeCount();
            
            int bw, bh;
            LoadRenderBufferNodes(c, "Per Preview No Offset", Nodes, bw, bh);
        } else {
            static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("Model group '%s' contains model '%s' that does not exist.", (const char *)GetFullName().c_str(), (const char *)mn[x].c_str());
            return false;
        }
    }

    //now have all the nodes for all the models
    float minx = 99999;
    float maxx = -1;
    float miny = 99999;
    float maxy = -1;
    
    int minChan = 9999999;
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        for (auto coord = (*it)->Coords.begin(); coord != (*it)->Coords.end(); coord++) {
            minx = std::min(minx, coord->screenX);
            miny = std::min(miny, coord->screenY);
            maxx = std::max(maxx, coord->screenX);
            maxy = std::max(maxy, coord->screenY);
        }
        if ((*it)->ActChan < minChan) {
            minChan = (*it)->ActChan;
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

    bool minimal = layout != "grid";

    float midX = (minx + maxx) / 2.0;
    float midY = (miny + maxy) / 2.0;
    if (!minimal) {
        minx = 0;
        miny = 0;
        maxx = std::max(maxx, (float)screenLocation.previewW);
        maxy = std::max(maxy, (float)screenLocation.previewH);
    }
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
        if (zeroBased) {
            (*it)->ActChan = (*it)->ActChan - minChan;
        }
    }

    BufferHt++;
    BufferWi++;
    if (!minimal) {
        BufferHt = std::max(BufferHt,(int)((float)screenLocation.previewH * hscale));
        BufferWi = std::max(BufferWi,(int)((float)screenLocation.previewW * wscale));
    }

    screenLocation.SetRenderSize(maxx - nminx + 1, maxy - nminy + 1);
    screenLocation.SetOffset(0.5, 0.5);

    SetMinMaxModelScreenCoordinates(screenLocation.previewW, screenLocation.previewH);
    return true;
}

ModelGroup::~ModelGroup()
{
}
void ModelGroup::AddModel(const std::string &name) {
    wxString newVal = ModelXml->GetAttribute("models", "");
    if (newVal.size() > 0) {
        newVal += ",";
    }
    newVal += name;
    ModelXml->DeleteAttribute("models");
    ModelXml->AddAttribute("models", newVal);
    Reset();
}
void ModelGroup::ModelRemoved(const std::string &oldName) {
    bool changed = false;
    wxString newVal;
    for (int x = 0; x < modelNames.size(); x++) {
        if (modelNames[x] == oldName) {
            changed = true;
        } else {
            if (x != 0) {
                newVal += ",";
            }
            newVal += modelNames[x];
        }
    }
    if (changed) {
        ModelXml->DeleteAttribute("models");
        ModelXml->AddAttribute("models", newVal);
        Reset();
    }
}
bool ModelGroup::ModelRenamed(const std::string &oldName, const std::string &newName) {
    bool changed = false;
    wxString newVal;
    for (int x = 0; x < modelNames.size(); x++) {
        if (modelNames[x] == oldName) {
            modelNames[x] = newName;
            changed = true;
        }
        if (x != 0) {
            newVal += ",";
        }
        newVal += modelNames[x];
    }
    if (changed) {
        ModelXml->DeleteAttribute("models");
        ModelXml->AddAttribute("models", newVal);
    }
    return changed;
}
void ModelGroup::CheckForChanges() const {
    unsigned long l = 0;
    for (auto it = models.begin(); it != models.end(); it++) {
        l += (*it)->GetChangeCount();
    }
    if (l != changeCount) {
        ModelGroup *group = (ModelGroup*)this;
        group->Reset();
    }
}

void ModelGroup::GetBufferSize(const std::string &tp, const std::string &transform, int &BufferWi, int &BufferHi) const {
    CheckForChanges();
    std::string type = tp;
    if (type.compare(0, 9, "Per Model") == 0) {
        type = "Default";
    }
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
        Model* m = modelManager[*it];
        ModelGroup *grp = dynamic_cast<ModelGroup*>(m);
        if (grp != nullptr) {
            int bw, bh;
            bw = bh = 0;
            models++;
            grp->GetBufferSize(HORIZ_PER_MODELSTRAND, "None", bw, bh);
            strands += bw;
            total += m->GetNodeCount();
            if (m->GetNodeCount() > maxNodes) {
                maxNodes = m->GetNodeCount();
            }
            maxWid = std::max(maxWid, m->GetDefaultBufferWi());
            maxHi = std::max(maxHi, m->GetDefaultBufferHt());
            maxStrandLen = std::max(maxStrandLen, bh);
        } else if (m != nullptr) {
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

static inline void SetCoords(NodeBaseClass::CoordStruct &it2, int x, int y, int maxX, int maxY, int scale) {
    if (maxX != -1) {
        x = x * maxX;
        x = x / scale;
    }
    if (maxY != -1) {
        y = y * maxY;
        y = y / scale;
    }
    it2.bufX = x;
    it2.bufY = y;
}

void ModelGroup::InitRenderBufferNodes(const std::string &tp,
                                       const std::string &transform,
                                       std::vector<NodeBaseClassPtr> &Nodes,
                                       int &BufferWi, int &BufferHi) const {
    CheckForChanges();
    std::string type = tp;
    if (type.compare(0, 9, "Per Model") == 0) {
        type = "Default";
    }
    if (type == "Default") {
        type = defaultBufferStyle;
    }
    BufferWi = BufferHi = 0;

    if (type == HORIZ_PER_MODEL) {
        for (auto it = modelNames.begin(); it != modelNames.end(); ++it) {
            Model* m = modelManager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "None", Nodes, x, y);
                y = 0;
                while (start < Nodes.size()) {
                    for (auto it2 = Nodes[start]->Coords.begin(); it2 != Nodes[start]->Coords.end(); ++it2) {
                        it2->bufX = BufferWi;
                        it2->bufY = y;
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
        for (auto it = modelNames.begin(); it != modelNames.end(); ++it) {
            Model* m = modelManager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "None", Nodes, x, y);
                y = 0;
                while (start < Nodes.size()) {
                    for (auto it2 = Nodes[start]->Coords.begin(); it2 != Nodes[start]->Coords.end(); ++it2) {
                        it2->bufY = BufferHi;
                        it2->bufX = y;
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
        for (auto it = modelNames.begin(); it != modelNames.end(); ++it) {
            Model* m = modelManager[*it];
            ModelGroup *grp = dynamic_cast<ModelGroup*>(m);
            int startBM = Nodes.size();
            if (grp != nullptr) {
                int bw, bh;
                bw = bh = 0;
                grp->InitRenderBufferNodes(type, "None", Nodes, bw, bh);
                for (int x = startBM; x < Nodes.size(); x++) {
                    for (auto it2 = Nodes[x]->Coords.begin(); it2 != Nodes[x]->Coords.end(); ++it2) {
                        if (horiz) {
                            it2->bufX += curS;
                        } else {
                            it2->bufY += curS;
                        }
                    }
                }
                if (horiz) {
                    curS += bw;
                } else {
                    curS += bh;
                }
            } else if (m != nullptr) {
                int bw, bh;
                bw = bh = 0;
                m->InitRenderBufferNodes(horiz ? "Horizontal Per Strand" : "Vertical Per Strand", "None", Nodes, bw, bh);
                for (int x = startBM; x < Nodes.size(); x++) {
                    for (auto it2 = Nodes[x]->Coords.begin(); it2 != Nodes[x]->Coords.end(); ++it2) {
                        if (horiz) {
                            SetCoords(*it2, it2->bufX + curS, it2->bufY, -1, BufferHi, bh);
                        } else {
                            SetCoords(*it2, it2->bufX, it2->bufY + curS, BufferWi, -1, bw);
                        }
                    }
                }
                if (horiz) {
                    curS += bw;
                } else {
                    curS += bh;
                }
            }
            if (m != nullptr) {
                int endBM = Nodes.size();
                if ((endBM - startBM) != m->GetNodeCount()) {
                    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                    logger_base.warn("Model group '%s' had problems creating render buffer for Per Strand/Model. Problem model '%s'.",
                                     (const char *)GetFullName().c_str(),
                                     (const char *)m->GetFullName().c_str());
                }
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHi);
    } else if (type == SINGLE_LINE) {
        BufferHi = 1;
        BufferWi = 0;
        for (auto it = modelNames.begin(); it != modelNames.end(); ++it) {
            Model* m = modelManager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Single Line", "None", Nodes, x, y);
                while (start < Nodes.size()) {
                    for (auto it2 = Nodes[start]->Coords.begin(); it2 != Nodes[start]->Coords.end(); ++it2) {
                        it2->bufX = BufferWi;
                        it2->bufY = 0;
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
        for (auto it = modelNames.begin(); it != modelNames.end(); ++it) {
            Model* m = modelManager[*it];
            if (m != nullptr) {
                int start = Nodes.size();
                int bw, bh;
                m->InitRenderBufferNodes("Default", "None", Nodes, bw, bh);
                if (bw != BufferWi || bh != BufferHi) {
                    //need to either scale or center
                    int offx = (BufferWi - bw)/2;
                    int offy = (BufferHi - bh)/2;
                    while (start < Nodes.size()) {
                        for (auto it2 = Nodes[start]->Coords.begin(); it2 != Nodes[start]->Coords.end(); ++it2) {
                            if (scale) {
                                it2->bufX = it2->bufX * (BufferWi/bw);
                                it2->bufY = it2->bufY * (BufferHi/bh);
                            } else {
                                it2->bufX += offx;
                                it2->bufY += offy;
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
