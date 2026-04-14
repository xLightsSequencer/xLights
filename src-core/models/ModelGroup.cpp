/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>

#include "ModelGroup.h"
#include "ModelManager.h"
#include "SingleLineModel.h"
#include "ModelScreenLocation.h"
#include "UtilFunctions.h"
#include "../XmlSerializer/XmlNodeKeys.h"
#include "../utils/ThreadUtils.h"

#include <log.h>

static const std::string HORIZ("Horizontal Stack");
static const std::string VERT("Vertical Stack");
static const std::string HORIZ_SCALED("Horizontal Stack - Scaled");
static const std::string VERT_SCALED("Vertical Stack - Scaled");
static const std::string HORIZ_PER_MODEL("Horizontal Per Model");
static const std::string VERT_PER_MODEL("Vertical Per Model");
static const std::string SINGLELINE_AS_PIXEL("Single Line Model As A Pixel");
static const std::string DEFAULT_AS_PIXEL("Default Model As A Pixel");
static const std::string HORIZ_PER_MODELSTRAND("Horizontal Per Model/Strand");
static const std::string VERT_PER_MODELSTRAND("Vertical Per Model/Strand");
static const std::string OVERLAY_CENTER("Overlay - Centered");
static const std::string OVERLAY_SCALED("Overlay - Scaled");
static const std::string SINGLE_LINE("Single Line");
static const std::string PER_MODEL_DEFAULT("Per Model Default");
static const std::string PER_MODEL_PER_PREVIEW("Per Model Per Preview");
static const std::string PER_MODEL_SINGLE_LINE("Per Model Single Line");
static const std::string PER_MODEL_DEFAULT_DEEP("Per Model Default Deep");
static const std::string PER_MODEL_VERT_STRAND("Per Model Vertical Per Strand");
static const std::string PER_MODEL_HORIZ_STRAND("Per Model Horizontal Per Strand");

std::vector<std::string> ModelGroup::GROUP_BUFFER_STYLES;

Model* ModelGroup::GetModel(std::string modelName) const
{
    for (const auto& it : models) {
        if (it->GetFullName() == modelName) {
            return it;
        }
    }

    return nullptr;
}

Model* ModelGroup::GetFirstModel() const
{
    for (const auto& it : models) {
        if (it->GetDisplayAs() != DisplayAsType::ModelGroup && it->GetDisplayAs() != DisplayAsType::SubModel) {
            return it;
        }
    }
    return nullptr;
}

// Gets a list of models in the group flattening out any groups and optionally removing any duplicates - submodels will be included if they are in the groups
std::list<Model*> ModelGroup::GetFlatModels(bool removeDuplicates, bool activeOnly) const
{
    std::list<Model*> res;

    for (const auto& it : modelNames) {
        Model* m = modelManager[it];
        if (m != nullptr) {
            if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
                auto mg = dynamic_cast<ModelGroup*>(m);
                if (mg != nullptr) {
                    for (const auto& it : mg->GetFlatModels(removeDuplicates, activeOnly)) {
                        if (!removeDuplicates || (std::find(begin(res), end(res), it) == end(res))) {
                            if (!activeOnly || m->IsActive()) {
                                res.push_back(it);
                            }
                        }
                    }
                }
            } else if (!removeDuplicates || (std::find(begin(res), end(res), m) == end(res))) {
                if (!activeOnly || m->IsActive()) {
                    res.push_back(m);
                }
            }
        }
    }
    return res;
}

bool ModelGroup::ContainsModelGroup(ModelGroup* mg)
{
    std::set<Model*> visited;
    visited.insert(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it)
    {
        if ((*it) != nullptr && (*it)->GetDisplayAs() == DisplayAsType::ModelGroup)
        {
            if (*it == mg)
            {
                found = true;
            }
            else
            {
                if (std::find(visited.begin(), visited.end(), *it) == visited.end())
                {
                    found |= dynamic_cast<ModelGroup*>(*it)->ContainsModelGroup(mg, visited);
                }
                else
                {
                    // already seen this group so dont follow
                }
            }
        }
    }

    return found;
}

bool ModelGroup::ContainsModelGroup(ModelGroup* mg, std::set<Model*>& visited)
{
    visited.insert(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it)
    {
        if ((*it) != nullptr && (*it)->GetDisplayAs() == DisplayAsType::ModelGroup)
        {
            if (*it == mg)
            {
                found = true;
            }
            else
            {
                if (visited.find(*it) == visited.end())
                {
                    found |= dynamic_cast<ModelGroup*>(*it)->ContainsModelGroup(mg, visited);
                }
                else
                {
                    // already seen this group so dont follow
                }
            }
        }
    }

    return found;
}

bool ModelGroup::DirectlyContainsModel(Model* m) const
{
    for (const auto& it : models) {
        if (m == it) {
            return true;
        }
    }

    return false;
}

bool ModelGroup::DirectlyContainsModel(std::string const& m) const
{
    return std::find(modelNames.begin(), modelNames.end(), m) != modelNames.end();
}

bool ModelGroup::ContainsModelOrSubmodel(const Model* m) const
{
    assert(m->GetDisplayAs() != DisplayAsType::ModelGroup);

    std::list<const Model*> visited;
    visited.push_back(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it) {
        if ((*it)->GetDisplayAs() == DisplayAsType::ModelGroup) {
            if (std::find(visited.begin(), visited.end(), *it) == visited.end()) {
                found |= dynamic_cast<ModelGroup*>(*it)->ContainsModelOrSubmodel(m, visited);
            } else {
                // already seen this group so dont follow
            }
        } else {
            if (StartsWith((*it)->GetFullName(), m->GetName())) {
                found = true;
            }
        }
    }

    return found;
}

bool ModelGroup::ContainsModel(const Model* m) const
{
    assert(m->GetDisplayAs() != DisplayAsType::ModelGroup);

    std::list<const Model*> visited;
    visited.push_back(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it)
    {
        if ((*it)->GetDisplayAs() == DisplayAsType::ModelGroup)
        {
            if (std::find(visited.begin(), visited.end(), *it) == visited.end())
            {
                found |= dynamic_cast<ModelGroup*>(*it)->ContainsModel(m, visited);
            }
            else
            {
                // already seen this group so dont follow
            }
        }
        else
        {
            if (m == *it)
            {
                found = true;
            }
        }
    }

    return found;
}

bool ModelGroup::ContainsModel(const Model* m, std::list<const Model*>& visited) const
{
    visited.push_back(this);

    bool found = false;
    for (const auto& it : models)
    {
        if (it->GetDisplayAs() == DisplayAsType::ModelGroup)
        {
            if (std::find(visited.begin(), visited.end(), it) == visited.end())
            {
                found |= dynamic_cast<ModelGroup*>(it)->ContainsModel(m, visited);
                if (found) break;
            }
            else
            {
                // already seen this group so dont follow
            }
        }
        else
        {
            if (m == it)
            {
                found = true;
                break;
            }
        }
    }

    return found;
}

bool ModelGroup::ContainsModelOrSubmodel(const Model* m, std::list<const Model*>& visited) const
{
    visited.push_back(this);

    bool found = false;
    for (const auto& it : models) {
        if (it->GetDisplayAs() == DisplayAsType::ModelGroup) {
            if (std::find(visited.begin(), visited.end(), it) == visited.end()) {
                found |= dynamic_cast<ModelGroup*>(it)->ContainsModelOrSubmodel(m, visited);
                if (found)
                    break;
            } else {
                // already seen this group so dont follow
            }
        } else {
            if (StartsWith(it->GetFullName(), m->GetName())) {
                found = true;
                break;
            }
        }
    }

    return found;
}

// Returns true if group only contains model and submodels of that model
bool ModelGroup::OnlyContainsModel(const std::string& name) const
{
    if (modelNames.size() == 0) return false;

    for (const auto& it : modelNames) {
        if (!StartsWith(it, name)) return false;
    }
    return true;
}

const std::vector<std::string> &ModelGroup::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            GROUP_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            GROUP_BUFFER_STYLES.push_back(HORIZ);
            GROUP_BUFFER_STYLES.push_back(VERT);
            GROUP_BUFFER_STYLES.push_back(HORIZ_SCALED);
            GROUP_BUFFER_STYLES.push_back(VERT_SCALED);
            GROUP_BUFFER_STYLES.push_back(HORIZ_PER_MODEL);
            GROUP_BUFFER_STYLES.push_back(VERT_PER_MODEL);
            GROUP_BUFFER_STYLES.push_back(HORIZ_PER_MODELSTRAND);
            GROUP_BUFFER_STYLES.push_back(VERT_PER_MODELSTRAND);
            GROUP_BUFFER_STYLES.push_back(OVERLAY_CENTER);
            GROUP_BUFFER_STYLES.push_back(OVERLAY_SCALED);
            GROUP_BUFFER_STYLES.push_back(SINGLELINE_AS_PIXEL);
            GROUP_BUFFER_STYLES.push_back(DEFAULT_AS_PIXEL);

            GROUP_BUFFER_STYLES.push_back(PER_MODEL_DEFAULT);
            GROUP_BUFFER_STYLES.push_back(PER_MODEL_PER_PREVIEW);
            GROUP_BUFFER_STYLES.push_back(PER_MODEL_SINGLE_LINE);

            GROUP_BUFFER_STYLES.push_back(PER_MODEL_DEFAULT_DEEP);

            GROUP_BUFFER_STYLES.push_back( PER_MODEL_VERT_STRAND);
            GROUP_BUFFER_STYLES.push_back( PER_MODEL_HORIZ_STRAND);
        }
    };
    static Initializer ListInitializationGuard;
    return GROUP_BUFFER_STYLES;
}

bool ModelGroup::AllModelsExist(pugi::xml_node node, const ModelManager& models)
{
    std::string nms = node.attribute("models").as_string();
    std::vector<std::string> mn;
    Split(nms, ',', mn, true);
    for (auto& it : mn) {
        if (!it.empty()) {
            Model* c = models.GetModel(it);
            if (c == nullptr) return false;
        }
    }
    return true;
}

bool ModelGroup::RemoveNonExistentModels(pugi::xml_node node, const std::set<std::string>& allmodels)
{
    bool changed = false;

    std::string models;
    std::string modelsRemoved;

    std::string nms = node.attribute("models").as_string();
    std::vector<std::string> mn;
    Split(nms, ',', mn, true);
    for (auto& mm : mn) {
        if (mm.find_first_of('/') != std::string::npos) {
            std::vector<std::string> smm;
            Split(mm, '/', smm, true);
            mm = "";
            for (auto& it2 : smm) {
                if (!mm.empty()) mm += "/";
                mm += it2;
            }
        }
        if (allmodels.find(mm) == allmodels.end()) {
            if (!modelsRemoved.empty()) modelsRemoved += ", ";
            modelsRemoved += mm;
            changed = true;
        } else {
            if (!models.empty()) models += ",";
            models += mm;
        }
    }

    if (changed && !modelsRemoved.empty()) {
        node.remove_attribute("models");
        node.append_attribute("models") = models;
    }
    return changed;
}

ModelGroup::ModelGroup(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    // Initialize basic state
    DisplayAs = DisplayAsType::ModelGroup;
    StringType = "RGB Nodes";
    selected = false;
    
    // Set default values
    m_gridSize = 400;
    m_xCentreOffset = 0;
    m_yCentreOffset = 0;
    m_defaultCamera = "2D";
    m_layout = "minimalGrid";
    centrex = 0;
    centrey = 0;
    centreDefined = false;
    layout_group = "Unassigned";
}

void ModelGroup::Accept(BaseObjectVisitor& visitor) const {
    visitor.Visit(*this);
}

void LoadRenderBufferNodes(Model *m, const std::string &type, const std::string &camera, std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHi, int stagger) {

    if (m == nullptr) return;

    // This skips over initialising inactive models so they also wont appear in groups.
    // There is a downside of doing this ... by never adding them to the group the size of the group changes which could change the appearance of some effects.
    // I am adding this now but I could well see us undoing this change if the solution is worse than the problem
    if (!m->IsActive())
        return; 

    if (m->GetDisplayAs() == DisplayAsType::ModelGroup)
    {
        ModelGroup *g = dynamic_cast<ModelGroup*>(m);
        if (g != nullptr) {
            for (const auto& it : g->ActiveModels()) {
                LoadRenderBufferNodes(it, type, camera, newNodes, bufferWi, bufferHi, stagger);
            }
        }
        else {
            m->InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi, stagger);
        }
    }
    else
    {
        m->InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi, stagger);
    }
}

int ModelGroup::GetGridSize() const
{
    return m_gridSize;
}

int ModelGroup::GetXCentreOffset() const
{
    return m_xCentreOffset;
}

int ModelGroup::GetYCentreOffset() const
{
    return m_yCentreOffset;
}

void ModelGroup::SetXCentreOffset( float cx )
{
    m_xCentreOffset = static_cast<int>(cx);
}

void ModelGroup::SetYCentreOffset( float cy )
{
    m_yCentreOffset = static_cast<int>(cy);
}

std::string ModelGroup::GetDefaultCamera() const
{
    return m_defaultCamera;
}

void ModelGroup::SetGridSize(int size)
{
    m_gridSize = size;
}

void ModelGroup::SetDefaultCamera(const std::string& camera)
{
    m_defaultCamera = camera;
}

void ModelGroup::SetLayout(const std::string& layout)
{
    m_layout = layout;
    
    // Update defaultBufferStyle based on layout
    defaultBufferStyle = layout;
    if (layout.compare(0, 9, "Per Model") == 0) {
        defaultBufferStyle = "Default";
    }
    if (layout == "grid" || layout == "minimalGrid") {
        defaultBufferStyle = "Default";
    } else if (layout == "vertical") {
        defaultBufferStyle = VERT_PER_MODEL;
    } else if (layout == "horizontal") {
        defaultBufferStyle = HORIZ_PER_MODEL;
    }
}

void ModelGroup::SetCentreX( float cx )
{
    centrex = cx;
}

void ModelGroup::SetCentreY( float cy )
{
    centrey = cy;
}

void ModelGroup::SetCentreDefined( bool defined )
{
    centreDefined = defined;
}

void ModelGroup::SetCentreMinx( int minx )
{
    this->minx = minx;
}

void ModelGroup::SetCentreMiny( int miny )
{
    this->miny = miny;
}

void ModelGroup::SetCentreMaxx( int maxx )
{
    this->maxx = maxx;
}

void ModelGroup::SetCentreMaxy( int maxy )
{
    this->maxy = maxy;
}

void ModelGroup::SetName(const std::string& newName)
{
    name = newName;
}

void ModelGroup::SetPreviewSize(int w, int h)
{
    screenLocation.previewW = w;
    screenLocation.previewH = h;
}

void ModelGroup::SetLayoutGroup(const std::string& group)
{
    layout_group = group;
}

void ModelGroup::SetBaseModels(const std::vector<std::string>& baseModels)
{
    m_baseModels = baseModels;
}

void ModelGroup::SetModels(const std::vector<std::string>& models)
{
    modelNames = models;
    ResetModels();
}

bool ModelGroup::RebuildBuffers() {
    // Rebuild buffer nodes and geometry from current member variables
    
    std::string layout = m_layout;
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
    activeModels.clear();
    changeCount = 0;
    
    // Build models and activeModels from modelNames
    int nc = 0;
    bool didnotexist = false;
    for (const auto& modelName : modelNames) {
        Model* c = modelManager.GetModel(modelName);
        if (c != nullptr) {
            models.push_back(c);
            if (c->IsActive()) {
                activeModels.push_back(c);
            }
            changeCount += c->GetChangeCount();
            nc += c->GetNodeCount();
        } else if (!modelName.empty()) {
            // model does not exist yet ... but it may soon
            didnotexist = true;
        }
    }

    if (didnotexist)
        return false;

    if (nc) {
        Nodes.reserve(nc);
    }

    for (Model* c : models) {
        int bw, bh;
        LoadRenderBufferNodes(c, "Per Preview No Offset", "2D", Nodes, bw, bh, 0);
    }

    bool minimal = layout != "grid";

    //now have all the nodes for all the models
    float minx = 99999;
    float maxx = -1;
    float miny = 99999;
    float maxy = -1;

    uint32_t minChan = 9999999;
    for (const auto& it : Nodes) {
        for (auto& coord : it->Coords) {
            minx = std::min(minx, coord.screenX);
            miny = std::min(miny, coord.screenY);
            maxx = std::max(maxx, coord.screenX);
            maxy = std::max(maxy, coord.screenY);
        }
        if (it->ActChan < minChan) {
            minChan = it->ActChan;
        }
    }

    float xminx = minx;
    float xmaxx = maxx;
    float xminy = miny;
    float xmaxy = maxy;

    if (miny < 0) {
        for (const auto& it : Nodes) {
            for (auto& coord : it->Coords) {
                coord.screenY -= miny;
            }
        }
        maxy -= miny;
        miny = 0;
    }
    if (minx < 0) {
        for (const auto& it : Nodes) {
            for (auto& coord : it->Coords) {
                coord.screenX -= minx;
            }
        }
        maxx -= minx;
        minx = 0;
    }

    float midX = (minx + maxx) / 2.0;
    float midY = (miny + maxy) / 2.0;
    if (!minimal) {
        minx = 0;
        miny = 0;
        maxx = std::max(maxx, (float)screenLocation.previewW);
        maxy = std::max(maxy, (float)screenLocation.previewH);
    }

    double hscale = m_gridSize / maxy;
    double wscale = m_gridSize / maxx;
    if (maxy < m_gridSize && maxx < m_gridSize) {
        hscale = 1.0;
        wscale = 1.0;
    }
    if (minimal) {
        if ((maxy-miny+1) < m_gridSize && (maxx-minx+1) < m_gridSize) {
            hscale = 1.0;
            wscale = 1.0;
        } else {
            hscale = m_gridSize / (maxy - miny + 1);
            wscale = m_gridSize / (maxx - minx + 1);
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

    for (const auto& it : Nodes) {
        for (auto& coord : it->Coords) {
            if (minimal) {
                coord.screenX = coord.screenX - minx;
                coord.screenY = coord.screenY - miny;
            }
            coord.bufX = coord.screenX * wscale;
            coord.bufY = coord.screenY * hscale;
            if (minimal) {
                coord.screenX = coord.screenX + minx - midX;
                coord.screenY = coord.screenY + miny - midY;
            } else {
                coord.screenX = coord.screenX - midX;
                coord.screenY = coord.screenY - midY;
            }
            maxx = std::max(maxx, (float)coord.screenX);
            maxy = std::max(maxy, (float)coord.screenY);
            nminx = std::min(nminx, (float)coord.screenX);
            nminy = std::min(nminy, (float)coord.screenY);
            BufferHt = std::max(BufferHt, (int)coord.bufY);
            BufferWi = std::max(BufferWi, (int)coord.bufX);
        }
    }

    BufferHt++;
    BufferWi++;
    if (!minimal) {
        BufferHt = std::max(BufferHt,(int)((float)screenLocation.previewH * hscale));
        BufferWi = std::max(BufferWi,(int)((float)screenLocation.previewW * wscale));
    }
    else
    {
        int offsetX;
        int offsetY;
        if( GetCentreDefined()) {
            float cx = GetCentreX();
            float cy = GetCentreY();
            if (xmaxx == xminx) {
                offsetX = 0;
            } else {
                offsetX = ((cx - ((xminx + xmaxx) / 2.0)) * 2000.0) / (xmaxx - xminx);
            }
            if (xmaxy == xminy) {
                offsetY = 0;
            } else {
                offsetY = ((cy - ((xminy + xmaxy) / 2.0)) * 2000.0) / (xmaxy - xminy);
            }
        } else {
            offsetX = GetXCentreOffset();
            offsetY = GetYCentreOffset();
        }

        int offx = (offsetX * BufferWi) / 1000;
        int offy = (offsetY * BufferHt) / 1000;

        BufferWi += std::abs(offx);
        BufferHt += std::abs(offy);

        if (offx > 0) offx = 0;
        if (offy > 0) offy = 0;

        if (offx < 0 || offy < 0) {
            for (const auto& it : Nodes) {
                for (auto& coord : it->Coords) {
                    coord.bufX -= offx;
                    coord.bufY -= offy;
                }
            }
        }
    }

    screenLocation.SetRenderSize(maxx - nminx + 1, maxy - nminy + 1);
    screenLocation.SetPosition(midX, midY);

    return true;
}

void ModelGroup::ResetModels()
{
    models.clear();
    activeModels.clear();
    
    for (const auto& modelName : modelNames) {
        Model* c = modelManager.GetModel(modelName);
        if (c != nullptr && c != this) {
            if (c->GetDisplayAs() == DisplayAsType::ModelGroup) {
                static_cast<ModelGroup*>(c)->ResetModels();
            }
            models.push_back(c);
            if (c->IsActive()) {
                activeModels.push_back(c);
            }
        }
    }
}

ModelGroup::~ModelGroup() {}

unsigned ModelGroup::GetFirstChannel() const
{
    unsigned first = 999999999;
    for (const auto& it : ModelNames())
    {
        Model* mm = modelManager.GetModel(it);
        if (mm != nullptr)
        {
            if (mm->GetFirstChannel() < first)
            {
                first = mm->GetFirstChannel();
            }
        }
    }

    if (first == 999999999)
    {
        first = 0;
    }

    return first;
}

unsigned int ModelGroup::GetLastChannel() const
{
    unsigned int last = 0;
    for (const auto& it : ModelNames())
    {
        Model* mm = modelManager.GetModel(it);
        if (mm != nullptr)
        {
            if (mm->GetLastChannel() > last)
            {
                last = mm->GetLastChannel();
            }
        }
    }

    return last;
}

void ModelGroup::AddModel(const std::string &name) {
    std::string trimmedName = Trim(name);
    
    // Add to modelNames if not already present
    if (std::find(modelNames.begin(), modelNames.end(), trimmedName) == modelNames.end()) {
        modelNames.push_back(trimmedName);
    }
    
    // Rebuild the models and activeModels vectors from modelNames
    ResetModels();
}

void ModelGroup::ModelRemoved(const std::string &oldName) {
    std::string trimmedOldName = Trim(oldName);
    
    // Remove all instances of the model from modelNames
    auto it = modelNames.begin();
    while (it != modelNames.end()) {
        if (Trim(*it) == trimmedOldName) {
            it = modelNames.erase(it);
        } else {
            ++it;
        }
    }
    
    // Rebuild the models and activeModels vectors from modelNames
    ResetModels();
}

bool ModelGroup::RemoveDuplicates()
{
    bool changed = false;
    for (int i = 0; i < (int)modelNames.size(); i++) {
        for (int j = i + 1; j < (int)modelNames.size(); j++) {
            if (modelNames[i] == modelNames[j]) {
                changed = true;
                auto it = begin(modelNames);
                std::advance(it, j);
                modelNames.erase(it);
                j--;
            }
        }
    }
    return changed;
}

bool ModelGroup::IsModelFromBase(const std::string& modelName) const
{
    for (const auto& baseModel : m_baseModels) {
        if (baseModel == modelName) {
            return true;
        }
    }
    return false;
}

bool ModelGroup::ModelRenamed(const std::string &oldName, const std::string &newName) {
    bool changed = false;
    
    for (int x = 0; x < (int)modelNames.size(); x++) {
        if (modelNames[x] == oldName) {
            modelNames[x] = newName;
            changed = true;
        }
        if (modelNames[x].find('/') != std::string::npos)
        {
            // this is a submodel
            std::string base = modelNames[x].substr(0, modelNames[x].find('/'));
            if (base == oldName)
            {
                auto startpos = modelNames[x].find('/');
                startpos++;
                modelNames[x] = newName + "/" + modelNames[x].substr(startpos);
                changed = true;
            }
        }
    }

    if (RemoveDuplicates()) {
        changed = true;
    }

    if (changed) {
        ResetModels();
    }
    return changed;
}

bool ModelGroup::SubModelRenamed(const std::string &oldName, const std::string &newName) {
    bool changed = false;
    
    for (int x = 0; x < (int)modelNames.size(); x++) {
        if (modelNames[x] == oldName) {
            modelNames[x] = newName;
            changed = true;
        }
    }
    
    if (changed) {
        ResetModels();
    }
    return changed;
}

bool ModelGroup::CheckForChanges() const {
    unsigned long l = 0;
    for (const auto& it : models) {
        ModelGroup *grp = dynamic_cast<ModelGroup*>(it);
        if (grp != nullptr) {
            grp->CheckForChanges();
        }
        l += it->GetChangeCount();
    }

    if (l != changeCount) {
        if (!IsMainThread()) {
            //calling reset on any thread other than the main thread is bad.  In theory, any changes to the group/model
            //would only be done on the main thread after an abortRender call so we shouldn't get here, but we are
            //seeing stack traces in crash reports that show otherwise so likely some abortRender calls are missing.
            return false;
        }
        
        // Rebuild buffers when contained models have changed
        ModelGroup *group = (ModelGroup*)this;
        if (group != nullptr) group->RebuildBuffers();
        return true;
    }
    return false;
}

void ModelGroup::GetBufferSize(const std::string &tp, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHt, int stagger) const {
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
    size_t maxNodes = 0;
    int total = 0;

    int totWid = 0;
    int totHi = 0;
    int maxWid = 0;
    int maxHi = 0;
    for (const auto& it : modelNames) {
        Model* m = modelManager[it];
        ModelGroup *grp = dynamic_cast<ModelGroup*>(m);
        if (grp != nullptr) {
            int bw, bh;
            bw = bh = 0;
            models++;
            grp->GetBufferSize(HORIZ_PER_MODELSTRAND, "2D", "None", bw, bh, stagger);
            strands += bw;
            total += m->GetNodeCount();
            if (m->GetNodeCount() > maxNodes) {
                maxNodes = m->GetNodeCount();
            }
            maxWid = std::max(maxWid, m->GetDefaultBufferWi());
            maxHi = std::max(maxHi, m->GetDefaultBufferHt());
            totWid += m->GetDefaultBufferWi();
            totHi += m->GetDefaultBufferHt();
            maxStrandLen = std::max(maxStrandLen, bh);
        } else if (m != nullptr && m->IsActive()) {
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
            totWid += m->GetDefaultBufferWi();
            totHi += m->GetDefaultBufferHt();
        }
    }
    if (type == HORIZ_PER_MODEL) {
        BufferWi = models;
        BufferHt = maxNodes;
    } else if (type == VERT_PER_MODEL) {
        BufferHt = models;
        BufferWi = maxNodes;
    } else if (type == HORIZ) {
        BufferHt = maxHi + (models - 1) * stagger;
        BufferWi = totWid;
    } else if (type == VERT) {
        BufferHt = totHi;
        BufferWi = maxWid + (models - 1) * stagger;
    } else if (type == HORIZ_SCALED) {
        BufferHt = maxHi;
        BufferWi = maxWid * models;
    } else if (type == VERT_SCALED) {
        BufferHt = maxHi * models;
        BufferWi = maxWid;
    } else if (type == SINGLELINE_AS_PIXEL) {
        BufferHt = 1;
        BufferWi = models;
    } else if (type == DEFAULT_AS_PIXEL) {
        Model::GetBufferSize("Per Preview", camera, "None", BufferWi, BufferHt, stagger);
    } else if (type == HORIZ_PER_MODELSTRAND) {
        BufferWi = strands;
        BufferHt = maxStrandLen;
    } else if (type == VERT_PER_MODELSTRAND) {
        BufferHt = strands;
        BufferWi = maxStrandLen;
    } else if (type == SINGLE_LINE) {
        BufferHt = 1;
        BufferWi = total;
    } else if (type == OVERLAY_CENTER || type == OVERLAY_SCALED) {
        BufferHt = maxHi;
        BufferWi = maxWid;
    } else {
        Model::GetBufferSize(type, camera, "None", BufferWi, BufferHt, stagger);
    }
    AdjustForTransform(transform, BufferWi, BufferHt);

    if (BufferWi == 0) BufferWi = 1;
    if (BufferHt == 0) BufferHt = 1;
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

// This code should be uncommented and use to find root cause of model groups that create buffers with different numbers of nodes
//void ModelGroup::TestNodeInit() const
//{
//    int x, y;
//    std::vector<NodeBaseClassPtr> mgNodesDefault;
//    Model::InitRenderBufferNodes("Default", "2D", "None", mgNodesDefault, x, y);

//    std::vector<NodeBaseClassPtr> mgNodesPerPreview;
//    Model::InitRenderBufferNodes("Per Preview", "2D", "None", mgNodesPerPreview, x, y);

//    int totalPerModelDefault = 0;
//    int totalSingleLine = 0;
//    for (const auto& it : modelNames) {
//        Model* m = modelManager[it];
//        if (m != nullptr) {
//            std::vector<NodeBaseClassPtr> nodesDefault;
//            m->InitRenderBufferNodes("Default", "2D", "None", nodesDefault, x, y);
//            totalPerModelDefault += nodesDefault.size();

//            std::vector<NodeBaseClassPtr> nodesSingleLine;
//            m->InitRenderBufferNodes("Single Line", "2D", "None", nodesSingleLine, x, y);
//            totalSingleLine += nodesSingleLine.size();

//            assert(nodesDefault.size() == nodesSingleLine.size());
//        }
//    }
//    assert(mgNodesDefault.size() == mgNodesPerPreview.size());
//    assert(mgNodesDefault.size() == totalPerModelDefault);
//    assert(mgNodesDefault.size() == totalSingleLine);
//}

void ModelGroup::InitRenderBufferNodes(const std::string& tp,
                                       const std::string& camera,
                                       const std::string& transform,
                                       std::vector<NodeBaseClassPtr>& Nodes,
                                       int& BufferWi, int& BufferHt, int stagger, bool deep) const
{
    CheckForChanges();
    std::string type = tp;
    if (type.compare(0, 9, "Per Model") == 0) {
        type = "Default";
    }
    if (type == "Default") {
        type = defaultBufferStyle;
    }
    BufferWi = 0;
    BufferHt = 0;

    if (type == HORIZ_PER_MODEL) {
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr && m->IsActive()) {
                int start = (int)Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y, stagger);
                y = 0;
                while (start < (int)Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufX = BufferWi;
                        it2.bufY = y;
                    }
                    y++;
                    start++;
                }
                if (y > BufferHt) {
                    BufferHt = y;
                }
                BufferWi++;
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == HORIZ) {
        int modelX = 0;
        int modelCount = 0;
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr && m->IsActive()) {
                int start = (int)Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y, stagger);
                while (start < (int)Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufX = it2.bufX + modelX;
                        if (stagger < 0) {
                            it2.bufY += ((int)modelNames.size() - modelCount - 1) * stagger;
                        } else {
                            it2.bufY += modelCount * stagger;
                        }
                    }
                    start++;
                }
                if (stagger < 0) {
                    if (y + ((int)modelNames.size() - modelCount - 1) * stagger > BufferHt) {
                        BufferHt = y + ((int)modelNames.size() - modelCount - 1) * stagger;
                    }
                }
                else
                {
                    if (y + modelCount * stagger > BufferHt) {
                        BufferHt = y + modelCount * stagger;
                    }
                }
                BufferWi += x;
                modelX += x;
            }
            ++modelCount;
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == VERT) {
        int modelY = 0;
        int modelCount = 0;
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr && m->IsActive()) {
                int start = (int)Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y, stagger);
                while (start < (int)Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufY = it2.bufY + modelY;
                        if (stagger < 0)
                        {
                            it2.bufX += ((int)modelNames.size() - modelCount - 1) * stagger;

                        } else {
                            it2.bufX += modelCount * stagger;
                        }
                    }
                    start++;
                }
                if (stagger < 0) {
                    if (x + ((int)modelNames.size() - modelCount - 1) * stagger > BufferWi) {
                        BufferWi = x + ((int)modelNames.size() - modelCount - 1) * stagger;
                    }
                } else {
                    if (x + modelCount * stagger > BufferWi) {
                        BufferWi = x + modelCount * stagger;
                    }
                }
                BufferHt += y;
                modelY += y;
            }
            ++modelCount;
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == HORIZ_SCALED) {
        int modelX = 0;
        int numOfModels = modelNames.size();
        if (numOfModels == 0) {
            BufferWi = 1;
            BufferHt = 1;
        } else {
            GetBufferSize(type, "2D", "None", BufferWi, BufferHt, stagger);
            int modBufferWi = BufferWi / numOfModels;
            for (const auto& it : modelNames) {
                Model* m = modelManager[it];
                if (m != nullptr && m->IsActive()) {
                    int start = (int)Nodes.size();
                    int x, y;
                    m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y, stagger);
                    while (start < (int)Nodes.size()) {
                        for (auto& it2 : Nodes[start]->Coords) {
                            it2.bufX = (double)it2.bufX * ((double)modBufferWi / (double)x) + (double)modelX;
                            it2.bufY = (double)it2.bufY * ((double)BufferHt / (double)y);
                        }
                        start++;
                    }
                    modelX += modBufferWi;
                }
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == VERT_SCALED) {
        int modelY = 0;
        int numOfModels = modelNames.size();

        if (numOfModels == 0) {
            BufferWi = 1;
            BufferHt = 1;
        } else {
            GetBufferSize(type, "2D", "None", BufferWi, BufferHt, stagger);
            int modBufferHt = BufferHt / numOfModels;
            for (const auto& it : modelNames) {
                Model* m = modelManager[it];
                if (m != nullptr && m->IsActive()) {
                    int start = (int)Nodes.size();
                    int x, y;
                    m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y, stagger);
                    while (start < (int)Nodes.size()) {
                        for (auto& it2 : Nodes[start]->Coords) {
                            it2.bufX = (double)it2.bufX * ((double)BufferWi / (double)x);
                            it2.bufY = (double)it2.bufY * ((double)modBufferHt / (double)y) + (double)modelY;
                        }
                        start++;
                    }
                    modelY += modBufferHt;
                }
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == VERT_PER_MODEL) {
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr && m->IsActive()) {
                int start = (int)Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y, stagger);
                y = 0;
                while (start < (int)Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufY = BufferHt;
                        it2.bufX = y;
                    }
                    y++;
                    start++;
                }
                if (y > BufferWi) {
                    BufferWi = y;
                }
                BufferHt++;
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == SINGLELINE_AS_PIXEL) {
        int outx = 0;
        BufferHt = 1;
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr && m->IsActive()) {
                int start = (int)Nodes.size();
                int x = 0;
                int y = 0;
                m->InitRenderBufferNodes("As Pixel", "2D", "None", Nodes, x, y, stagger);
                while (start < (int)Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufY = 0;
                        it2.bufX = outx;
                    }
                    start++;
                }
                outx++;
            }
        }
        BufferWi = outx;
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == DEFAULT_AS_PIXEL) {
        int start = Nodes.size();
        Model::InitRenderBufferNodes("Per Preview", "2D", "None", Nodes, BufferWi, BufferHt, stagger);
        for (auto modelName : modelNames) {
            Model* c = modelManager.GetModel(modelName);
            if (c != nullptr && c->IsActive()) {
                int cx = 0;
                int cy = 0;
                int cnt = 0;
                //find the middle
                for (size_t x = 0; x < c->GetNodeCount(); x++) {
                    for (auto& coord : Nodes[start + x]->Coords) {
                        cx += coord.bufX;
                        cy += coord.bufY;
                        cnt++;
                    }
                }
                if (cnt != 0) {
                    cx /= cnt;
                    cy /= cnt;
                    for (size_t x = 0; x < c->GetNodeCount(); x++) {
                        for (auto& coord : Nodes[start + x]->Coords) {
                            coord.bufX = cx;
                            coord.bufY = cy;
                        }
                    }
                }
                start += c->GetNodeCount();
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == HORIZ_PER_MODELSTRAND || type == VERT_PER_MODELSTRAND) {
        GetBufferSize(type, "2D", "None", BufferWi, BufferHt, stagger);
        bool horiz = type == HORIZ_PER_MODELSTRAND;
        int curS = 0;
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            ModelGroup* grp = dynamic_cast<ModelGroup*>(m);
            int startBM = (int)Nodes.size();
            if (grp != nullptr) {
                int bw, bh;
                bw = bh = 0;
                grp->InitRenderBufferNodes(type, "2D", "None", Nodes, bw, bh, stagger);
                for (int x = startBM; x < (int)Nodes.size(); x++) {
                    for (auto& it2 : Nodes[x]->Coords) {
                        if (horiz) {
                            it2.bufX += curS;
                        } else {
                            it2.bufY += curS;
                        }
                    }
                }
                if (horiz) {
                    curS += bw;
                } else {
                    curS += bh;
                }
            } else if (m != nullptr && m->IsActive()) {
                int bw, bh;
                bw = bh = 0;
                m->InitRenderBufferNodes(type, "2D", "None", Nodes, bw, bh, stagger);
                for (int x = startBM; x < (int)Nodes.size(); x++) {
                    for (auto& it2 : Nodes[x]->Coords) {
                        if (horiz) {
                            SetCoords(it2, it2.bufX + curS, it2.bufY, -1, BufferHt, bh);
                        } else {
                            SetCoords(it2, it2.bufX, it2.bufY + curS, BufferWi, -1, bw);
                        }
                    }
                }
                if (horiz) {
                    curS += bw;
                } else {
                    curS += bh;
                }
            }
            if (m != nullptr && m->IsActive()) {
                int endBM = (int)Nodes.size();
                if ((endBM - startBM) != (int)m->GetNodeCount()) {
                    
                    spdlog::warn("Model group '{}' had problems creating render buffer for Per Strand/Model. Problem model '{}'.",
                                     (const char*)GetFullName().c_str(),
                                     (const char*)m->GetFullName().c_str());
                }
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == SINGLE_LINE) {
        BufferHt = 1;
        BufferWi = 0;
        if (deep) {
            for (const auto& it : GetFlatModels(false, true)) {
                Model* m = it;
                assert(m != nullptr);
                if (!m->IsActive()) continue;
                int start = (int)Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Single Line", "2D", "None", Nodes, x, y, stagger);
                while (start < (int)Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufX = BufferWi;
                        it2.bufY = 0;
                    }
                    start++;
                    BufferWi++;
                }
            }
        } else {
            for (const auto& it : modelNames) {
                Model* m = modelManager[it];
                if (m != nullptr && m->IsActive()) {
                    int start = (int)Nodes.size();
                    int x, y;
                    m->InitRenderBufferNodes("Single Line", "2D", "None", Nodes, x, y, stagger);
                    while (start < (int)Nodes.size()) {
                        for (auto& it2 : Nodes[start]->Coords) {
                            it2.bufX = BufferWi;
                            it2.bufY = 0;
                        }
                        start++;
                        BufferWi++;
                    }
                }
            }
        }

        // Buffer widths of zero cause crashes elsewhere in effects so force it to be at least 1 wide
        if (BufferWi < 1)
            BufferWi = 1;

        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == OVERLAY_CENTER || type == OVERLAY_SCALED) {
        bool scale = type == OVERLAY_SCALED;
        GetBufferSize(type, "2D", "None", BufferWi, BufferHt, stagger);
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr && m->IsActive()) {
                int start = (int)Nodes.size();
                int bw, bh;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, bw, bh, stagger);
                if (bw != BufferWi || bh != BufferHt) {
                    //need to either scale or center
                    int offx = (BufferWi - bw) / 2;
                    int offy = (BufferHt - bh) / 2;
                    while (start < (int)Nodes.size()) {
                        for (auto& it2 : Nodes[start]->Coords) {
                            if (scale) {
                                it2.bufX = (double)it2.bufX * ((double)BufferWi / (double)bw);
                                it2.bufY = (double)it2.bufY * ((double)BufferHt / (double)bh);
                            } else {
                                it2.bufX += offx;
                                it2.bufY += offy;
                            }
                        }
                        start++;
                    }
                }
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else {
        if (camera == "2D" && type == "Per Preview") {
            Nodes.clear();
        }
        Model::InitRenderBufferNodes(type, camera, transform, Nodes, BufferWi, BufferHt, stagger);
    }
    // Buffer needs at least one pixel as several effects will divide by the Width/Height and such
    // which can result in divide by 0.
    // Some of the buffer styles above (several of the PerModel) will result in one of these
    // being 0 as there aren't models to iterate over.   We'll set to the default 1x1 buffer.
    // Note:  this also matches the behavior of GetBufferSize(...) above
    if (BufferWi == 0) {
        BufferWi = 1;
    }
    if (BufferHt == 0) {
        BufferHt = 1;
    }
}
