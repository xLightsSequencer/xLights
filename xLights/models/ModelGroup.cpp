/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/sstream.h>

#include "ModelGroup.h"
#include "ModelManager.h"
#include "SingleLineModel.h"
#include "ModelScreenLocation.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

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
        if (it->GetDisplayAs() != "ModelGroup" && it->GetDisplayAs() != "SubModel") {
            return it;
        }
    }
    return nullptr;
}

// Gets a list of models in the group flattening out any groups and optionally removing any duplicates - submodels will be included if they are in the groups
std::list<Model*> ModelGroup::GetFlatModels(bool removeDuplicates) const
{
    std::list<Model*> res;

    for (const auto& it : modelNames) {
        Model* m = modelManager[it];
        if (m != nullptr) {
            if (m->GetDisplayAs() == "ModelGroup") {
                auto mg = dynamic_cast<ModelGroup*>(m);
                if (mg != nullptr) {
                    for (const auto& it : mg->GetFlatModels(removeDuplicates)) {
                        if (!removeDuplicates || (std::find(begin(res), end(res), it) == end(res))) {
                            res.push_back(it);
                        }
                    }
                }
            } else if (!removeDuplicates || (std::find(begin(res), end(res), m) == end(res))) {
                res.push_back(m);
            }
        }
    }
    return res;
}

bool ModelGroup::ContainsModelGroup(ModelGroup* mg)
{
    std::list<Model*> visited;
    visited.push_back(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it)
    {
        if ((*it) != nullptr && (*it)->GetDisplayAs() == "ModelGroup")
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

bool ModelGroup::ContainsModelGroup(ModelGroup* mg, std::list<Model*>& visited)
{
    visited.push_back(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it)
    {
        if ((*it) != nullptr && (*it)->GetDisplayAs() == "ModelGroup")
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

bool ModelGroup::DirectlyContainsModel(Model* m) const
{
    for (const auto& it : models) {
        if (m == it) {
            return true;
        }
    }

    return false;
}

bool ModelGroup::DirectlyContainsModel(std::string const& m ) const
{
    return std::find(modelNames.begin(), modelNames.end(), m) != modelNames.end();
}

bool ModelGroup::ContainsModel(Model* m) const
{
    wxASSERT(m->GetDisplayAs() != "ModelGroup");

    std::list<const Model*> visited;
    visited.push_back(this);

    bool found = false;
    for (auto it = models.begin(); !found && it != models.end(); ++it)
    {
        if ((*it)->GetDisplayAs() == "ModelGroup")
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

bool ModelGroup::ContainsModel(Model* m, std::list<const Model*>& visited) const
{
    visited.push_back(this);

    bool found = false;
    for (const auto& it : models)
    {
        if (it->GetDisplayAs() == "ModelGroup")
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

// Returns true if group only contains model and submodels of that model
bool ModelGroup::OnlyContainsModel(const std::string& name) const
{
    if (modelNames.size() == 0) return false;

    for (const auto& it : modelNames) {
        if (!StartsWith(it, name)) return false;
    }
    return true;
}

std::string ModelGroup::SerialiseModelGroup(const std::string& forModel) const
{
    wxXmlDocument new_doc;
    new_doc.SetRoot(new wxXmlNode(*GetModelXml()));

    std::string nmns;
    auto mns = wxSplit(new_doc.GetRoot()->GetAttribute("models"), ',');
    for (auto& it : mns) {
        if(!it.StartsWith(forModel)) continue;
        if (nmns != "") nmns += ",";
        it.Replace(forModel + "/", "EXPORTEDMODEL/");
        nmns += it;
    }
    new_doc.GetRoot()->DeleteAttribute("models");
    new_doc.GetRoot()->AddAttribute("models", nmns);
    wxStringOutputStream stream;
    new_doc.Save(stream);
    wxString s = stream.GetString();
    return (s.SubString(s.Find("\n") + 1, s.Length()) + "\n").ToStdString(); // skip over xml format header
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

bool ModelGroup::AllModelsExist(wxXmlNode* node, const ModelManager& models)
{
    wxArrayString mn = wxSplit(node->GetAttribute("models"), ',');
    for (auto& it : mn) {
        if (it != "")
        {
            Model* c = models.GetModel(it.Trim(true).Trim(false).ToStdString());
            if (c == nullptr) return false;
        }
    }
    return true;
}

bool ModelGroup::RemoveNonExistentModels(wxXmlNode* node, const std::list<std::string>& allmodels)
{
    bool changed = false;

    std::string models;
    std::string modelsRemoved;

    wxString name = node->GetAttribute("name", "").Trim(false).Trim(true);
    wxArrayString mn = wxSplit(node->GetAttribute("models", ""), ',');

    for (auto& it : mn) {
        auto mm = it.Trim(true).Trim(false);
        if (mm.Contains("/"))
        {
            auto smm = wxSplit(mm, '/');
            mm = "";
            for (auto& it2 : smm)
            {
                if (mm != "") mm += "/";
                mm += it2.Trim(true).Trim(false);
            }
        }
        if (std::find(allmodels.begin(), allmodels.end(), mm) == allmodels.end())
        {
            if (modelsRemoved != "") modelsRemoved += ", ";
            modelsRemoved += mm;
            changed = true;
        }
        else
        {
            if (models != "") models += ",";
            models += mm;
        }
    }

    if (changed && modelsRemoved != "") {
        node->DeleteAttribute("models");
        node->AddAttribute("models", models);

        // I have removed the warn code ... mainly because there is really not anything a user can or should do so why tell them
    }

    return changed;
}

ModelGroup::ModelGroup(wxXmlNode *node, const ModelManager &m, int w, int h) : ModelWithScreenLocation(m)
{
    ModelXml = node;
    screenLocation.previewW = w;
    screenLocation.previewH = h;
    Reset();
}

ModelGroup::ModelGroup(wxXmlNode* node, const ModelManager& m, int w, int h, const std::string& mgname, const std::string& mname) : ModelWithScreenLocation(m)
{
    ModelXml = new wxXmlNode(*node);
    ModelXml->DeleteAttribute("name");
    ModelXml->AddAttribute("name", mgname);
    screenLocation.previewW = w;
    screenLocation.previewH = h;

    // We have to fix the model name before we reset otherwise it will fail
    auto mn = wxSplit(ModelXml->GetAttribute("models"), ',');
    std::string nmns;
    for (auto& it : mn) {
        if (nmns != "") nmns += ",";
        it.Replace("EXPORTEDMODEL", mname);
        nmns += it;
    }
    ModelXml->DeleteAttribute("models");
    ModelXml->AddAttribute("models", nmns);

    Reset();
}

void LoadRenderBufferNodes(Model *m, const std::string &type, const std::string &camera, std::vector<NodeBaseClassPtr> &newNodes, int &bufferWi, int &bufferHi) {

    if (m == nullptr) return;

    // This skips over initialising inactive models so they also wont appear in groups.
    // There is a downside of doing this ... by never adding them to the group the size of the group changes which could change the appearance of some effects.
    // I am adding this now but I could well see us undoing this change if the solution is worse than the problem
    if (!m->IsActive())
        return; 

    if (m->GetDisplayAs() == "ModelGroup")
    {
        ModelGroup *g = dynamic_cast<ModelGroup*>(m);
        if (g != nullptr) {
            for (const auto& it : g->Models()) {
                LoadRenderBufferNodes(it, type, camera, newNodes, bufferWi, bufferHi);
            }
        }
        else {
            m->InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi);
        }
    }
    else
    {
        m->InitRenderBufferNodes(type, camera, "None", newNodes, bufferWi, bufferHi);
    }
}

int ModelGroup::GetGridSize() const
{
    return wxAtoi(ModelXml->GetAttribute("GridSize", "400"));
}

int ModelGroup::GetXCentreOffset() const
{
    return wxAtoi(ModelXml->GetAttribute("XCentreOffset", "0"));
}

int ModelGroup::GetYCentreOffset() const
{
    return wxAtoi(ModelXml->GetAttribute("YCentreOffset", "0"));
}

bool ModelGroup::Reset(bool zeroBased) {
    this->zeroBased = zeroBased;
    selected = false;
    name = ModelXml->GetAttribute("name").Trim(true).Trim(false).ToStdString();

    DisplayAs = "ModelGroup";
    StringType = "RGB Nodes";

    layout_group = ModelXml->GetAttribute("LayoutGroup", "Unassigned");
    int gridSize = wxAtoi(ModelXml->GetAttribute("GridSize", "400"));
    int offsetX = wxAtoi(ModelXml->GetAttribute("XCentreOffset", "0"));
    int offsetY = wxAtoi(ModelXml->GetAttribute("YCentreOffset", "0"));
    modelTagColour = wxColour(ModelXml->GetAttribute("TagColour", "Black"));
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
    int nc = 0;
    for (int x = 0; x < mn.size(); x++) {
        Model *c = modelManager.GetModel(mn[x].Trim(true).Trim(false).ToStdString());
        if (c != nullptr) {
            modelNames.push_back(c->GetFullName());
            models.push_back(c);
            changeCount += c->GetChangeCount();
            nc += c->GetNodeCount();
        }
        else if (mn[x] == "")
        {
            // silently ignore blank models
        }
        else
        {
            // model does not exist yet ... but it may soon
            return false;
        }
    }

    if (nc) {
        Nodes.reserve(nc);
    }

    for (Model *c : models) {
        int bw, bh;
        LoadRenderBufferNodes(c, "Per Preview No Offset", "2D", Nodes, bw, bh);
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
        if (zeroBased) {
            it->ActChan = it->ActChan - minChan;
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
        int offx = (offsetX * BufferWi) / 1000;
        int offy = (offsetY * BufferHt) / 1000;

        BufferWi += std::abs(offx);
        BufferHt += std::abs(offy);

        if (offx > 0) offx = 0;
        if (offy > 0) offy = 0;

        for (const auto& it : Nodes) {
            for (auto& coord : it->Coords) {
                coord.bufX -= offx;
                coord.bufY -= offy;
            }
        }
    }

    screenLocation.SetRenderSize(maxx - nminx + 1, maxy - nminy + 1);
    screenLocation.SetPosition(BufferWi / 2.0f, BufferHt / 2.0f);

    return true;
}

void ModelGroup::ResetModels()
{
    models.clear();
    wxArrayString mn = wxSplit(ModelXml->GetAttribute("models"), ',');
    for (int x = 0; x < mn.size(); x++) {
        Model *c = modelManager.GetModel(mn[x].Trim(true).Trim(false).ToStdString());
        if (c != nullptr && c != this) {
            if (c->GetDisplayAs() == "ModelGroup") {
                static_cast<ModelGroup*>(c)->ResetModels();
            }
            models.push_back(c);
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
    wxString newVal = ModelXml->GetAttribute("models", "");
    if (newVal.size() > 0) {
        newVal += ",";
    }
    newVal += Trim(name);
    ModelXml->DeleteAttribute("models");
    ModelXml->AddAttribute("models", newVal);
    Reset();
}

void ModelGroup::ModelRemoved(const std::string &oldName) {
    bool changed = false;
    auto on = Trim(oldName);
    wxString newVal;
    for (int x = 0; x < modelNames.size(); x++) {
        if (Trim(modelNames[x]) == on) {
            changed = true;
        } else {
            if (x != 0) {
                newVal += ",";
            }
            newVal += Trim(modelNames[x]);
        }
    }
    if (changed) {
        ModelXml->DeleteAttribute("models");
        ModelXml->AddAttribute("models", newVal);
        Reset();
    }
}

bool ModelGroup::RemoveDuplicates()
{
    bool changed = false;
    for (int i = 0; i < modelNames.size(); i++) {
        for (int j = i + 1; j < modelNames.size(); j++) {
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

bool ModelGroup::ModelRenamed(const std::string &oldName, const std::string &newName) {
    bool changed = false;
    wxString newVal;
    for (int x = 0; x < modelNames.size(); x++) {
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
        if (x != 0) {
            newVal += ",";
        }
        newVal += modelNames[x];
    }

    if (RemoveDuplicates()) {
        changed = true;
    }

    if (changed) {
        ModelXml->DeleteAttribute("models");
        ModelXml->AddAttribute("models", newVal);
        ResetModels();
    }
    return changed;
}

bool ModelGroup::SubModelRenamed(const std::string &oldName, const std::string &newName) {
    bool changed = false;
    wxString newVal = "";
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
        // this is ugly ... it is casting away the const-ness of this
        ModelGroup *group = (ModelGroup*)this;
        if (group != nullptr) group->Reset();
        return true;
    }
    return false;
}

void ModelGroup::GetBufferSize(const std::string &tp, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHt) const {
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
            grp->GetBufferSize(HORIZ_PER_MODELSTRAND, "2D", "None", bw, bh);
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
        BufferHt = maxHi;
        BufferWi = totWid;
    } else if (type == VERT) {
        BufferHt = totHi;
        BufferWi = maxWid;
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
        Model::GetBufferSize("Per Preview", camera, "None", BufferWi, BufferHt);
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
        Model::GetBufferSize(type, camera, "None", BufferWi, BufferHt);
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

//            wxASSERT(nodesDefault.size() == nodesSingleLine.size());
//        }
//    }
//    wxASSERT(mgNodesDefault.size() == mgNodesPerPreview.size());
//    wxASSERT(mgNodesDefault.size() == totalPerModelDefault);
//    wxASSERT(mgNodesDefault.size() == totalSingleLine);
//}

void ModelGroup::InitRenderBufferNodes(const std::string& tp,
                                       const std::string& camera,
                                       const std::string& transform,
                                       std::vector<NodeBaseClassPtr>& Nodes,
                                       int& BufferWi, int& BufferHt, bool deep) const
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
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y);
                y = 0;
                while (start < Nodes.size()) {
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
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y);
                while (start < Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufX = it2.bufX + modelX;
                    }
                    start++;
                }
                if (y > BufferHt) {
                    BufferHt = y;
                }
                BufferWi += x;
                modelX += x;
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == VERT) {
        int modelY = 0;
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y);
                while (start < Nodes.size()) {
                    for (auto& it2 : Nodes[start]->Coords) {
                        it2.bufY = it2.bufY + modelY;
                    }
                    start++;
                }
                if (x > BufferWi) {
                    BufferWi = x;
                }
                BufferHt += y;
                modelY += y;
            }
        }
        ApplyTransform(transform, Nodes, BufferWi, BufferHt);
    } else if (type == HORIZ_SCALED) {
        int modelX = 0;
        int numOfModels = modelNames.size();
        if (numOfModels == 0) {
            BufferWi = 1;
            BufferHt = 1;
        } else {
            GetBufferSize(type, "2D", "None", BufferWi, BufferHt);
            int modBufferWi = BufferWi / numOfModels;
            for (const auto& it : modelNames) {
                Model* m = modelManager[it];
                if (m != nullptr) {
                    int start = Nodes.size();
                    int x, y;
                    m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y);
                    while (start < Nodes.size()) {
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
            GetBufferSize(type, "2D", "None", BufferWi, BufferHt);
            int modBufferHt = BufferHt / numOfModels;
            for (const auto& it : modelNames) {
                Model* m = modelManager[it];
                if (m != nullptr) {
                    int start = Nodes.size();
                    int x, y;
                    m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y);
                    while (start < Nodes.size()) {
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
            if (m != nullptr) {
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, x, y);
                y = 0;
                while (start < Nodes.size()) {
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
            if (m != nullptr) {
                int start = Nodes.size();
                int x = 0;
                int y = 0;
                m->InitRenderBufferNodes("As Pixel", "2D", "None", Nodes, x, y);
                while (start < Nodes.size()) {
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
        Model::InitRenderBufferNodes("Per Preview", "2D", "None", Nodes, BufferWi, BufferHt);
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
        GetBufferSize(type, "2D", "None", BufferWi, BufferHt);
        bool horiz = type == HORIZ_PER_MODELSTRAND;
        int curS = 0;
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            ModelGroup* grp = dynamic_cast<ModelGroup*>(m);
            int startBM = Nodes.size();
            if (grp != nullptr) {
                int bw, bh;
                bw = bh = 0;
                grp->InitRenderBufferNodes(type, "2D", "None", Nodes, bw, bh);
                for (int x = startBM; x < Nodes.size(); x++) {
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
            } else if (m != nullptr) {
                int bw, bh;
                bw = bh = 0;
                m->InitRenderBufferNodes(horiz ? "Horizontal Per Strand" : "Vertical Per Strand", "2D", "None", Nodes, bw, bh);
                for (int x = startBM; x < Nodes.size(); x++) {
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
            if (m != nullptr) {
                int endBM = Nodes.size();
                if ((endBM - startBM) != m->GetNodeCount()) {
                    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
                    logger_base.warn("Model group '%s' had problems creating render buffer for Per Strand/Model. Problem model '%s'.",
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
            for (const auto& it : GetFlatModels(false)) {
                Model* m = it;
                wxASSERT(m != nullptr);
                int start = Nodes.size();
                int x, y;
                m->InitRenderBufferNodes("Single Line", "2D", "None", Nodes, x, y);
                while (start < Nodes.size()) {
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
                if (m != nullptr) {
                    int start = Nodes.size();
                    int x, y;
                    m->InitRenderBufferNodes("Single Line", "2D", "None", Nodes, x, y);
                    while (start < Nodes.size()) {
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
        GetBufferSize(type, "2D", "None", BufferWi, BufferHt);
        for (const auto& it : modelNames) {
            Model* m = modelManager[it];
            if (m != nullptr) {
                int start = Nodes.size();
                int bw, bh;
                m->InitRenderBufferNodes("Default", "2D", "None", Nodes, bw, bh);
                if (bw != BufferWi || bh != BufferHt) {
                    //need to either scale or center
                    int offx = (BufferWi - bw) / 2;
                    int offy = (BufferHt - bh) / 2;
                    while (start < Nodes.size()) {
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
        Model::InitRenderBufferNodes(type, camera, transform, Nodes, BufferWi, BufferHt);
    }

    //wxASSERT(BufferWi != 0);
    //wxASSERT(BufferHt != 0);
}
