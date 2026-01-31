/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <vector>

#include "CustomModel.h"
#include "../CustomModelDialog.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
#include "outputs/Controller.h"
#include "UtilFunctions.h"
#include "../ExternalHooks.h"
#include "outputs/OutputManager.h"
#include "../ModelPreview.h"
#include "RulerObject.h"
#include "../XmlSerializer/XmlSerializer.h"

#include <log4cpp/Category.hh>

CustomModel::CustomModel(const ModelManager &manager) : ModelWithScreenLocation(manager)
{
    _depth = 1;
    _strings = 1;
    screenLocation.SetSupportsZScaling(true);
    screenLocation.SetStartOnXAxis(true);
    _locations.resize(1);
    _locations.back().resize(1);
    _locations.back().back().resize(1);
}

CustomModel::~CustomModel()
{
    _locations.clear();
}

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class CustomModelDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    CustomModelDialogAdapter(CustomModel* model, OutputManager* om) :
        wxPGEditorDialogAdapter(), m_model(model), _outputManager(om)
    {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
        wxPGProperty* WXUNUSED(property)) override
    {
        m_model->SaveDisplayDimensions();
        auto oldAutoSave = m_model->GetModelManager().GetXLightsFrame()->_suspendAutoSave;
        m_model->GetModelManager().GetXLightsFrame()->_suspendAutoSave = true; // because we will tamper with model we need to suspend autosave
        CustomModelDialog dlg(propGrid, _outputManager);
        dlg.Setup(m_model);
        bool res = false;
        if (dlg.ShowModal() == wxID_OK) {
            dlg.Save(m_model);
            m_model->RestoreDisplayDimensions();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            res = true;
        }
        else {
            m_model->RestoreDisplayDimensions();
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            m_model->GetModelManager().GetXLightsFrame()->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CustomModel::CancelCustomData");
        }
        m_model->GetModelManager().GetXLightsFrame()->_suspendAutoSave = oldAutoSave;
        return res;
    }
protected:
    CustomModel* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

class CustomModelProperty : public wxStringProperty
{
public:
    CustomModelProperty(CustomModel* m,
        OutputManager* om,
        const wxString& label,
        const wxString& name,
        const wxString& value) :
        wxStringProperty(label, name, value), m_model(m), _outputManager(om)
    {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override
    {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new CustomModelDialogAdapter(m_model, _outputManager);
    }
protected:
    CustomModel* m_model = nullptr;
    OutputManager* _outputManager = nullptr;
};

void CustomModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxPGProperty* p = grid->Append(new CustomModelProperty(this, outputManager, "Model Data", "CustomData", CLICK_TO_EDIT));
    grid->LimitPropertyEditing(p);

    p = grid->Append(new wxUIntProperty("# Strings", "CustomModelStrings", _strings));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_strings == 1) {
        // cant set start node
    }
    else {
        wxString nm = StartNodeAttrName(0);

        p = grid->Append(new wxStringProperty("Start Nodes", "ModelIndividualStartNodes", ""));
        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, _hasIndivNodes ? _indivStartNodes[0] : 1));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (_hasIndivNodes) {
            int c = _strings;
            for (int x = 0; x < c; ++x) {
                nm = StartNodeAttrName(x);
                int v = _indivStartNodes[x];
                if (v < 1) v = ComputeStringStartNode(x);
                if (v > NodesPerString()) v = NodesPerString();
                if (x == 0) {
                    psn->SetValue(v);
                }
                else {
                    grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                }
            }
        }
        else {
            psn->Enable(false);
        }
    }

    wxStopWatch sw;
    p = grid->Append(new wxImageFileProperty("Background Image", "CustomBkgImage", _custom_background));

    if (sw.Time() > 500)
        logger_base.debug("        Adding background image property (%s) to model %s really slow: %lums", (const char*)_custom_background.c_str(), (const char*)name.c_str(), sw.Time());

    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                        ";*.webp"
                                        "|All files (*.*)|*.*");
}

int CustomModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("CustomData" == event.GetPropertyName()) {
        if (grid->GetPropertyByName("CustomBkgImage")->GetValue() != _custom_background) {
            grid->GetPropertyByName("CustomBkgImage")->SetValue(wxVariant(_custom_background));
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CustomModel::OnPropertyGridChange::CustomData");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CustomModel::OnPropertyGridChange::CustomData");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CustomModel::OnPropertyGridChange::CustomData");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CustomModel::OnPropertyGridChange::CustomData");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "CustomModel::OnPropertyGridChange::CustomData");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CustomModel::OnPropertyGridChange::CustomData");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "CustomModel::OnPropertyGridChange::CustomData");
        return 0;
    } else if ("CustomBkgImage" == event.GetPropertyName()) {
        _custom_background = event.GetValue().GetString();
        ObtainAccessToURL(_custom_background);
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CustomModel::OnPropertyGridChange::CustomBkgImage");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CustomModel::OnPropertyGridChange::CustomBkgImage");
        return 0;
    } else if ("CustomModelStrings" == event.GetPropertyName()) {
        int old_string_count = _strings;
        _strings = event.GetValue().GetInteger();
        _hasIndivNodes = _strings > 1;
        if (_strings != old_string_count && _hasIndivNodes) {
            _indivStartNodes.resize(_strings);
            for (int x = 0; x < _strings; x++) {
                _indivStartNodes[x] = ComputeStringStartNode(x);
            }
        }
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CustomModel::OnPropertyGridChange::CustomModelStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CustomModel::OnPropertyGridChange::CustomModelStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CustomModel::OnPropertyGridChange::CustomModelStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CustomModel::OnPropertyGridChange::CustomModelStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "CustomModel::OnPropertyGridChange::CustomModelStrings");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualStartNodes.String")) {
        wxString s = event.GetPropertyName().substr(strlen("ModelIndividualStartNodes.String"));
        int string = wxAtoi(s) - 1;
        int value = event.GetValue().GetInteger();
        if (value < 1) value = 1;
        if (value > NodesPerString()) value = NodesPerString();
        _indivStartNodes[string] = value;
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CustomModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CustomModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "CustomModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "CustomModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "CustomModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

std::tuple<int, int, int> FindNode(int node, const std::vector<std::vector<std::vector<int>>>& locations)
{
    for (int l = 0; l < locations.size(); l++) {
        for (int r = 0; r < locations[l].size(); r++) {
            for (int c = 0; c < locations[l][r].size(); c++) {
                if (locations[l][r][c] == node + 1) {
                    return { l, r, c };
                }
            }
        }
    }
    wxASSERT(false);
    return { -1,-1,-1 };
}

int CustomModel::GetStrandLength(int strand) const
{
    return Nodes.size();
}

int CustomModel::MapToNodeIndex(int strand, int node) const
{
    return node;
}

void CustomModel::UpdateModel(int width, int height, int depth, const std::vector<std::vector<std::vector<int>>>& modelData) {
    parm1 = width;
    parm2 = height;
    _depth = depth;
    _locations = modelData;
    Setup();
}

void CustomModel::InitModel()
{
    InitCustomMatrix();
    //CopyBufCoord2ScreenCoord();
    screenLocation.SetRenderSize(parm1, parm2, _depth);
    if (_depth > 1) {
        screenLocation.SetPerspective2D(0.1f); // if i dont do this you cant see the back nodes in 2D
    }
}

void CustomModel::SetCustomWidth(long w)
{
    parm1 = w;
}

void CustomModel::SetCustomHeight(long h)
{
    parm2 = h;
}

void CustomModel::SetCustomDepth(long d)
{
    _depth = d;
    if (_depth < 1) _depth = 1;
}

const std::string CustomModel::GetCustomData() const
{
    // TODO:  Delete ToCustom  and move its contents here
    return CustomModel::ToCustomModel(_locations);
}

const std::string CustomModel::GetCompressedData() const
{
    // TODO:  Delete ToCompressed and move its contents here
    return CustomModel::ToCompressed(_locations);
}

void CustomModel::SetCustomData(const std::vector<std::vector<std::vector<int>>>& data)
{
    _locations = data;
    Setup();
}

void CustomModel::SetCustomBackground(std::string background)
{
    _custom_background = background;
}

bool CustomModel::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (FileExists(_custom_background)) {
        if (!frame->IsInShowFolder(_custom_background)) {
            _custom_background = frame->MoveToShowFolder(_custom_background, wxString(wxFileName::GetPathSeparator()) + "Images");
            Setup();
            rc = true;
        }
    }

    return Model::CleanupFileLocations(frame) || rc;
}

bool CustomModel::IsAllNodesUnique() const
{
    if (Nodes.size() == 0)
        return false; // this is a special case where i want to treat it like it is not unique
    for (const auto& n : Nodes) {
        if (n->Coords.size() > 1)
            return false;
    }
    return true;
}

std::list<std::string> CustomModel::GetFileReferences()
{
    std::list<std::string> res;
    if (FileExists(_custom_background)) {
        res.push_back(_custom_background);
    }
    return res;
}

void CustomModel::SetNumStrings(int strings)
{
    _strings = strings;
}

void CustomModel::SetStringStartChannels(int NumberOfStrings, int StartChannel, int ChannelsPerString)
{
    int maxval = GetCustomMaxChannel();
    // fix NumberOfStrings
    if (SingleNode) {
        NumberOfStrings = maxval;
    }
    else {
        ChannelsPerString = maxval * GetNodeChannelCount(StringType) / _strings;
    }

    if (_strings == 1) {
        Model::SetStringStartChannels(NumberOfStrings, StartChannel, ChannelsPerString);
    }
    else {
        stringStartChan.clear();
        stringStartChan.resize(_strings);
        for (int i = 0; i < _strings; i++) {
            int node = _indivStartNodes[i];
            if (node == 0) {
                node = ((ChannelsPerString * i) / GetNodeChannelCount(StringType)) + 1;
            }
            if (node > maxval) node = maxval;
            stringStartChan[i] = (StartChannel - 1) + (node - 1) * GetNodeChannelCount(StringType);
        }
    }
}

int CustomModel::NodesPerString() const
{
    int nodes = GetChanCount() / std::max(GetChanCountPerNode(), 1);

    int ts = GetSmartTs();
    if (ts <= 1) {
        return nodes;
    }
    else {
        return nodes * ts;
    }
}

static std::vector<std::string> CUSTOM_BUFFERSTYLES =
{
    "Default",
    "Per Preview",
    "Single Line",
    "As Pixel",
    "Stacked X Horizontally",
    "Stacked Y Horizontally",
    "Stacked Z Horizontally",
    "Stacked X Vertically",
    "Stacked Y Vertically",
    "Stacked Z Vertically",
    "Overlaid X",
    "Overlaid Y",
    "Overlaid Z",
    "Unique X and Y X",
    "Unique X and Y Y",
    "Unique X and Y Z",
};

const std::vector<std::string>& CustomModel::GetBufferStyles() const
{
    return CUSTOM_BUFFERSTYLES;
}

void CustomModel::GetBufferSize(const std::string& tp, const std::string& camera, const std::string& transform, int& BufferWi, int& BufferHi, int stagger) const
{
    int width = parm1;
    int height = parm2;
    int depth = _depth;
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;

    if ((SingleNode || SingleChannel) && IsMultiCoordsPerNode())
    {
        BufferWi = GetCustomMaxChannel();
        BufferHi = 1;
    }
    else if (StartsWith(type, "Per Preview") || type == "Single Line" || type == "As Pixel" ||
        type == "Horizontal Per Strand" || type == "Vertical Per Strand" ||
        type == "Horizontal Per Model/Strand" || type == "Vertical Per Model/Strand") {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    }
    else if (type == "Stacked X Horizontally") {
        BufferHi = height;
        BufferWi = width * depth;
    }
    else if (type == "Stacked Y Horizontally") {
        BufferHi = depth;
        BufferWi = width * height;
    }
    else if (type == "Default" || type == "Stacked Z Horizontally") {
        BufferHi = height;
        BufferWi = width * depth;
    }
    else if (type == "Stacked X Vertically") {
        BufferHi = height * width;
        BufferWi = depth;
    }
    else if (type == "Stacked Y Vertically") {
        BufferHi = height * depth;
        BufferWi = width;
    }
    else if (type == "Stacked Z Vertically") {
        BufferWi = width;
        BufferHi = depth * height;
    }
    else if (type == "Overlaid X") {
        BufferWi = depth;
        BufferHi = height;
    }
    else if (type == "Overlaid Y") {
        BufferWi = width;
        BufferHi = depth;
    }
    else if (type == "Overlaid Z") {
        BufferWi = width;
        BufferHi = height;
    }
    else if (type == "Unique X and Y X") {
        BufferWi = height * width;
        BufferHi = depth * width;
    }
    else if (type == "Unique X and Y Y") {
        BufferWi = width * height;
        BufferHi = depth * height;
    }
    else if (type == "Unique X and Y Z") {
        BufferWi = width * depth;
        BufferHi = height * depth;
    }
    else {
        wxASSERT(false);
    }

    AdjustForTransform(transform, BufferWi, BufferHi);
}

void CustomModel::InitRenderBufferNodes(const std::string& tp, const std::string& camera, const std::string& transform, std::vector<NodeBaseClassPtr>& Nodes, int& BufferWi, int& BufferHi, int stagger, bool deep) const
{
    int width = parm1;
    int height = parm2;
    int depth = _depth;
    std::string type = tp.starts_with("Per Model ") ? tp.substr(10) : tp;

    wxASSERT(width > 0 && height > 0 && depth > 0);

    int startNodeSize = Nodes.size();
    Model::InitRenderBufferNodes(type, camera, transform, Nodes, BufferWi, BufferHi, stagger);

    if ((SingleChannel || SingleNode) && IsMultiCoordsPerNode()) {
        // I am not 100% about this change but it makes sense to me
        // While the custom model may have a height and width if it is single channel then the render buffer really should be Nodes x 1
        // and all nodes should point to one cell.
        // Without this change effects like twinkle do really strange things
        BufferWi = Nodes.size() - startNodeSize;
        BufferHi = 1;
        int x = 0;
        while (startNodeSize < Nodes.size()) {
            for (auto& it2 : Nodes[startNodeSize]->Coords) {
                it2.bufX = x;
                it2.bufY = 0;
            }
            x++;
            startNodeSize++;
        }
        return;
    }
    else if (SingleChannel || SingleNode) {
        return;
    }

    if (StartsWith(type, "Per Preview") || type == "Single Line" || type == "As Pixel" ||
        StartsWith(type, "Horizontal Per ") || StartsWith(type, "Vertical Per ")) {
        return;
    }

    GetBufferSize(type, camera, transform, BufferWi, BufferHi, stagger);
    if (type == "Stacked X Horizontally") {
        for (auto n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = depth - std::get<0>(loc) - 1 + std::get<2>(loc) * depth;
            Nodes[n]->Coords[0].bufY = height - std::get<1>(loc) - 1;
        }
    }
    else if (type == "Stacked Y Horizontally") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc) + std::get<1>(loc) * width;
            Nodes[n]->Coords[0].bufY = std::get<0>(loc);
        }
    }
    else if (type == "Default" || type == "Stacked Z Horizontally") {
        // dont need to do anything
    }
    else if (type == "Stacked X Vertically") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = depth - std::get<0>(loc) - 1;
            Nodes[n]->Coords[0].bufY = std::get<1>(loc) + height * std::get<2>(loc);
        }
    }
    else if (type == "Stacked Y Vertically") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc);
            Nodes[n]->Coords[0].bufY = std::get<0>(loc) + depth * std::get<1>(loc);
        }
    }
    else if (type == "Stacked Z Vertically") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc);
            Nodes[n]->Coords[0].bufY = std::get<1>(loc) + depth * std::get<0>(loc);
        }
    }
    else if (type == "Overlaid X") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = depth - std::get<0>(loc) - 1;
            Nodes[n]->Coords[0].bufY = height - std::get<1>(loc) - 1;
        }
    }
    else if (type == "Overlaid Y") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc);
            Nodes[n]->Coords[0].bufY = std::get<0>(loc);
        }
    }
    else if (type == "Overlaid Z") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc);
            Nodes[n]->Coords[0].bufY = height - std::get<1>(loc) - 1;
        }
    }
    else if (type == "Unique X and Y X") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = depth - std::get<0>(loc) - 1 + std::get<2>(loc) * depth;
            Nodes[n]->Coords[0].bufY = std::get<1>(loc) + std::get<2>(loc) * height;
        }
    }
    else if (type == "Unique X and Y Y") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc) + std::get<1>(loc) * width;
            Nodes[n]->Coords[0].bufY = std::get<0>(loc) + std::get<1>(loc) * depth;
        }
    }
    else if (type == "Unique X and Y Z") {
        for (size_t n = 0; n < Nodes.size(); n++) {
            auto loc = FindNode(n, _locations);
            Nodes[n]->Coords[0].bufX = std::get<2>(loc) + std::get<0>(loc) * width;
            Nodes[n]->Coords[0].bufY = std::get<1>(loc) + (height - std::get<1>(loc) - 1) * height;
        }
    }
    else {
        wxASSERT(false);
    }
}

int CustomModel::GetCustomMaxChannel() const
{
    int maxval = 0;
    int layer = 0;
    for (const auto& l : _locations) {
        size_t row = 0;
        for (const auto& r : l) {
            size_t col = 0;
            for (const auto& c : r) {
                if (_locations[layer][row][col] > maxval) {
                    maxval = _locations[layer][row][col];
                }
                ++col;
            }
            ++row;
        }
        ++layer;
    }
    return maxval;
}

std::string CustomModel::CustomModelToCompressed(const std::string& customModel)
{
    return CustomModel::ToCompressed(XmlSerialize::ParseCustomModel(customModel));
}

std::string CustomModel::CompressedToCustomModel(const std::string& compressed)
{
    return CustomModel::ToCustomModel(XmlSerialize::ParseCompressed(compressed));
}

std::string CustomModel::ToCompressed(const std::vector<std::vector<std::vector<int>>>& model) {

    // we only compress if nodes to cells < 20%
    int nodes = 0;
    int cells = model.size() * model[0].size() * model[0][0].size();
    for (const auto& l : model) {
		for (const auto& r : l) {
			for (const auto& c : r) {
				if (c >= 0) {
					nodes++;
				}
			}
		}
	}

    if (nodes > 0.80 * cells)
        return "";

    int layers = model.size();
    std::string compressed = "";
    for (int l = 0; l < model.size(); l++) {
		for (int r = 0; r < model[l].size(); r++) {
			for (int c = 0; c < model[l][r].size(); c++) {
				if (model[l][r][c] >= 0) {
					if (!compressed.empty()) {
						compressed += ";";
					}
					compressed += std::to_string(model[l][r][c]) + "," + std::to_string(r) + "," + std::to_string(c);
					if (layers > 1) {
						compressed += "," + std::to_string(l);
					}
				}
			}
		}
	}

    return compressed;
}

std::string CustomModel::ToCustomModel(const std::vector<std::vector<std::vector<int>>>& model) {
    std::string customModel = "";
	for (int l = 0; l < model.size(); l++) {
		if (!customModel.empty()) {
			customModel += "|";
		}
		for (int r = 0; r < model[l].size(); r++) {
			if (r > 0) {
				customModel += ";";
			}
			for (int c = 0; c < model[l][r].size(); c++) {
				if (c > 0) {
					customModel += ",";
				}
				if (model[l][r][c] >= 0) {
					customModel += std::to_string(model[l][r][c]);
				}
			}
		}
	}
	return customModel;
}

void CustomModel::InitCustomMatrix() {

    uint32_t depth = _locations.size();
    uint32_t height = _locations[0].size();
    uint32_t width = _locations[0][0].size();

    // find the maximum node
    int maxval = 0;
    for (const auto& l : _locations) {
		for (const auto& r : l) {
			for (const auto& c : r) {
				if (c >= 0) {
					maxval = std::max(maxval, c);
				}
			}
		}
	}

    std::vector<int> nodemap;
    nodemap.resize(maxval + 1, -1);

    int32_t firstStartChan = 999999999;
    for (auto it : stringStartChan) {
        firstStartChan = std::min(it, firstStartChan);
    }

    int cpn = -1;

    // now populate the nodes
    size_t layer = 0;
    for (const auto& l : _locations) {
        size_t row = 0;
        for (const auto& r : l) {
            size_t col = 0;
            for (const auto& c : r) {
                if (c > 0) {
                    int idx = c - 1;//index is zero based
                    // is node already defined in map?
                    if (nodemap[idx] < 0) {
                        // unmapped - so add a node
                        nodemap[idx] = Nodes.size();
                        SetNodeCount(1, 0, rgbOrder); // this creates a node of the correct class
                        Nodes.back()->StringNum = idx;
                        if (cpn == -1) {
                            cpn = GetChanCountPerNode();
                        }
                        Nodes.back()->ActChan = firstStartChan + idx * cpn;
                        if (idx < nodeNames.size() && !nodeNames[idx].empty()) {
                            Nodes.back()->SetName(nodeNames[idx]);
                        } else {
                            Nodes.back()->SetName("Node " + std::to_string(idx + 1));
                        }

                        Nodes.back()->AddBufCoord(layer * ((float)width) + col, ((float)height) - row - 1);
                        auto& cc = Nodes[nodemap[idx]]->Coords.back();
                        cc.screenX = (float)col - ((float)width) / 2.0f;
                        cc.screenY = ((float)height) - (float)row - 1.0f - ((float)height) / 2.0f;
                        cc.screenZ = depth - (float)layer - 1.0f - depth / 2.0f;
                    } else {
                        // mapped - so add a coord to existing node
                        Nodes[nodemap[idx]]->AddBufCoord(layer * ((float)width) + col, ((float)height) - row - 1);
                        auto& c = Nodes[nodemap[idx]]->Coords.back();
                        c.screenX = (float)col - ((float)width) / 2.0f;
                        c.screenY = ((float)height) - (float)row - 1.0f - ((float)height) / 2.0f;
                        c.screenZ = depth - (float)layer - 1.0f - depth / 2.0f;
                    }
                }
                ++col;
            }
            ++row;
        }
        ++layer;
    }

    for (size_t x = 0; x < Nodes.size(); x++) {
        for (size_t y = x + 1; y < Nodes.size(); y++) {
            if (Nodes[y]->StringNum < Nodes[x]->StringNum) {
                Nodes[x].swap(Nodes[y]);
            }
        }
    }

    for (size_t x = 0; x < Nodes.size(); x++) {
        if (Nodes[x]->GetName().empty()) {
            Nodes[x]->SetName(GetNodeName(Nodes[x]->StringNum));
        }
    }

    // we have 2 sources of truth for the width, height and depth but we take the parm settings rather than the data
    //SetBufferSize(height, width * depth);
    SetBufferSize(parm2, parm1 * _depth);
    if (screenLocation.RenderDp < 10.0f) {
        screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
    }
}

int CustomModel::ComputeStringStartNode(int x) const
{
    if (x == 0) return 1;

    int strings = GetNumPhysicalStrings();
    int nodes = GetNodeCount();
    float nodesPerString = (float)nodes / (float)strings;

    return (int)(x * nodesPerString + 1);
}

int CustomModel::GetCustomNodeStringNumber(int node) const
{
    if (_strings == 1) {
        return 1;
    }

    int stringStart = -1;
    int string = -1;
    for (int i = 0; i < _strings; i++) {
        int startNode = 1;
        if (_hasIndivNodes) {
            startNode = _indivStartNodes[i];
        } else {
            startNode = ComputeStringStartNode(i);
        }
        if (node >= startNode && startNode >= stringStart) {
            string = i;
            stringStart = startNode;
        }
    }
    return string + 1;
}

std::string CustomModel::GetNodeName(size_t x, bool def) const {
    if (x < Nodes.size()) {
        return Nodes[x]->GetName();
    }
    if (def) {
        return std::string("Node ") + std::to_string(x + 1);
    }
    return "";
}

#define PERFORMANCE_IMPACT_SIZE 300
std::list<std::string> CustomModel::CheckModelSettings()
{
    std::list<std::string> res;

    // check for no nodes
    if (GetNodeCount() == 0) {
        res.push_back(wxString::Format("    ERR: Custom model '%s' has no nodes defined.", GetName()));
    }

    if (!xLightsFrame::IsCheckSequenceOptionDisabled("CustomSizeCheck")) {
        if (parm1 > PERFORMANCE_IMPACT_SIZE || parm2 > PERFORMANCE_IMPACT_SIZE || _depth > PERFORMANCE_IMPACT_SIZE) {
            float pop = ((float)GetNodeCount() * 100) / (float)(parm1 * parm2);
            if (pop < 10.0) { // allow models which have more than 1 in 10 cells used as these likely need to be that large
                res.push_back(wxString::Format("    WARN: Custom model '%s' dimensions are really large (%ld x %ld x %d : Nodes %u => %0.2f%%). This may impact xLights render performance.", GetName(), parm1, parm2, _depth, GetNodeCount(), pop).ToStdString());
            }
        }
    }

    // if multiple strings then check the start nodes
    // one string should start at 1
    // all should be less than the number of nodes
    // there should be no duplicates\        wxString nm = StartNodeAttrName(0);
    auto nm = StartNodeAttrName(0);
    if (_strings > 1 && _hasIndivNodes) {
        bool oneFound = false;
        std::vector<int> prevStart;
        int nodes = GetChanCount() / GetChanCountPerNode();
        for (int i = 0; i < _strings; i++) {
            nm = StartNodeAttrName(i);
            auto val = _indivStartNodes[i];
            if (val == 1) {
                oneFound = true;
            }
            if (std::find(begin(prevStart), end(prevStart), val) != end(prevStart)) {
                res.push_back(wxString::Format("    ERR: Custom model '%s' String %d starts at a node %d which has already been used by another string.", GetName(), i, val).ToStdString());
            }
            if (val == 0 || val > nodes) {
                res.push_back(wxString::Format("    ERR: Custom model '%s' String %d starts at a node %d outside the node count %d in the model.", GetName(), i+1, val, nodes).ToStdString());
            }
            prevStart.push_back(val);
        }
        if (!oneFound)             {
            res.push_back(wxString::Format("    ERR: Custom model '%s' Multiple strings but none starting at node 1.", GetName()).ToStdString());
        }
    }

    // check for node gaps
    int maxn = 0;
    for (size_t ii = 0; ii < GetNodeCount(); ii++) {
        int nn = GetNodeStringNumber(ii);
        if (nn > maxn) maxn = nn;
    }
    maxn++;
    int chssize = (maxn + 1) * sizeof(int);
    //logger_base.debug("    CheckSequence: Checking custom model %d nodes", maxn);
    int* chs = (int*)malloc(chssize);
    if (chs == nullptr) {
        res.push_back(wxString::Format("    WARN: Could not check Custom model '%s' for missing nodes. Error allocating memory for %d nodes.", GetName(), maxn).ToStdString());
    }
    else {
        memset(chs, 0x00, chssize);

        for (size_t ii = 0; ii < GetNodeCount(); ii++) {
            int nn = GetNodeStringNumber(ii);
            chs[nn + 1]++;
        }

        long lastStart = -1;
        for (int ii = 1; ii <= maxn; ii++) {
            if (chs[ii] == 0) {
                if (lastStart == -1) {
                    lastStart = ii;
                }
            }
            else {
                if (lastStart != -1) {
                    if (lastStart == ii - 1) {
                        res.push_back(wxString::Format("    WARN: Custom model '%s' missing node %d.", GetName(), lastStart).ToStdString());
                    }
                    else {
                        res.push_back(wxString::Format("    WARN: Custom model '%s' missing nodes %d-%d.", GetName(), lastStart, ii - 1).ToStdString());
                    }
                    lastStart = -1;
                }
            }
        }

        int multinodecount = 0;
        for (size_t ii = 0; ii < GetNodeCount(); ii++) {
            std::vector<wxPoint> pts;
            GetNodeCoords(ii, pts);
            if (pts.size() > 1) {
                multinodecount++;
            }
        }

        // >0% but less than 10% multi-nodes ... these may be accidental duplicates
        if (multinodecount > 0 && multinodecount < 0.1 * maxn) {
            for (size_t ii = 0; ii < GetNodeCount(); ii++) {
                std::vector<wxPoint> pts;
                GetNodeCoords(ii, pts);
                if (pts.size() > 1) {
                    res.push_back(wxString::Format("    WARN: Custom model '%s' %s node has %d instances but multi instance nodes are rare in this model so this may be unintended.",
                        GetName(),
                        Ordinal(ii + 1),
                        (int)pts.size()).ToStdString());
                }
            }
        }

        free(chs);
    }

    res.splice(res.end(), Model::CheckModelSettings());
    return res;
}

int CustomModel::NodesPerString(int string) const
{
    if (_strings == 1) {
        return NodesPerString();
    }

    int32_t lowestStartChannel = 2000000000;
    for (int i = 0; i < _strings; i++) {
        if (stringStartChan[i] < lowestStartChannel) lowestStartChannel = stringStartChan[i];
    }

    int32_t ss = stringStartChan[string];
    int32_t len = GetChanCount() - (ss - lowestStartChannel);
    for (int i = 0; i < _strings; i++) {
        if (i != string) {
            if (stringStartChan[i] > ss && len > stringStartChan[i] - ss) {
                len = stringStartChan[i] - ss;
            }
        }
    }
    return len / GetNodeChannelCount(StringType);
}

std::string CustomModel::ChannelLayoutHtml(OutputManager* outputManager) {
    size_t NodeCount = GetNodeCount();
    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi, 0);
    std::string direction = "n/a";

    int32_t sc;
    Controller* c = outputManager->GetController(this->GetFirstChannel()+ 1, sc);

    std::string html = "<html><body><table border=0>";
    html += "<tr><td>Name:</td><td>" + name + "</td></tr>";
    html += "<tr><td>Display As:</td><td>" + DisplayAs + "</td></tr>";
    html += "<tr><td>String Type:</td><td>" + StringType + "</td></tr>";
    html += "<tr><td>Start Corner:</td><td>" + direction + "</td></tr>";
    html += wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>", (int)NodeCount);
    html += wxString::Format("<tr><td>Width:</td><td>%d</td></tr>", BufferWi);
    html += wxString::Format("<tr><td>Height:</td><td>%d</td></tr>", BufferHt);
    if (c != nullptr)
        html += wxString::Format("<tr><td>Controller:</td><td>%s</td></tr>", c->GetLongDescription());
    if ("" != GetControllerProtocol())
    {
        html += wxString::Format("<tr><td>Pixel protocol:</td><td>%s</td></tr>", GetControllerProtocol());
        if (_strings == 1)
        {
            html += wxString::Format("<tr><td>Controller Connection:</td><td>%d</td></tr>", GetControllerPort());
        }
        else
        {
            html += wxString::Format("<tr><td>Controller Connection:</td><td>%d-%d</td></tr>", GetControllerPort(), GetControllerPort() + _strings - 1);
        }
    }
    html += "</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    std::string data = GetCustomData();
    if (data == "") {
        html += "<tr><td>No custom data</td></tr>";
    }
    else {
        std::vector<std::vector<std::vector<wxString>>> _data;
        int cols = parm1;
        wxArrayString layers = wxSplit(data, '|');
        for (auto l : layers) {
            std::vector<std::vector<wxString>> ll;
            wxArrayString rows = wxSplit(l, ';');
            for (auto r : rows) {
                std::vector<wxString> rr;
                wxArrayString columns = wxSplit(r, ',');
                for (auto c : columns) {
                    rr.push_back(c);
                }
                while (rr.size() < cols) rr.push_back("");
                ll.push_back(rr);
            }
            // This should never happen but i have seen files where it did so lets just pad it and not crash
            while (ll.size() < parm2) {
                std::vector<wxString> rr;
                while (rr.size() < cols) rr.push_back("");
                ll.push_back(rr);
            }
            _data.push_back(ll);
        }

        for (int r = 0; r < parm2; r++) {
            html += "<tr>";
            for (int l = 0; l < _depth; l++) {
                for (int c = 0; c < parm1; c++) {
                    wxString value = _data[l][r][c];
                    if (!value.IsEmpty() && value != "0") {
                        if (_strings == 1) {
                            if( IsDarkMode() ) {
                                html += wxString::Format("<td bgcolor='#962B09'>n%s</td>", value);
                            } else {
                                html += wxString::Format("<td bgcolor='#ADD8E6'>n%s</td>", value);
                            }
                        }
                        else {
                            int string = GetCustomNodeStringNumber(wxAtoi(value));
                            wxString bgcolor;
                            switch (string % 4)
                            {
                            case 0:
                                bgcolor = IsDarkMode() ? "#7a577a" : "#eed1a4"; // purple / yellow
                                break;
                            case 1:
                                bgcolor = IsDarkMode() ? "#3f7c85" : "#8aa2bb"; // teal / blue
                                break;
                            case 2:
                                bgcolor = IsDarkMode() ? "#520120" : "#ec9396"; // maroon / red
                                break;
                            case 3:
                                bgcolor = IsDarkMode() ? "#08403e" : "#86d0c3"; // dark teal / green
                                break;
                            }
                            html += wxString::Format("<td bgcolor='" + bgcolor + "'>n%ss%d</td>",value, string);
                        }
                    }
                    else {
                        html += "<td>&nbsp&nbsp&nbsp</td>";
                    }
                }
            }
            html += "</tr>";
        }
    }

    html += "</table></body></html>";
    return html;
}

bool point_compare(const wxPoint first, const wxPoint second)
{
    if (first.x == second.x) {
        return first.y < second.y;
    }

    return first.x < second.x;
}

void RemoveDuplicatePixels(std::list<std::list<wxPoint>>& chs)
{
    std::list<wxPoint> flat;
    std::list<wxPoint> duplicates;

    for (const auto& ch : chs) {
        for (const auto& it : ch) {
            flat.push_back(wxPoint(it.x, it.y));
        }
    }

    flat.sort(point_compare);

    for (auto it = flat.begin(); it != flat.end(); ++it) {
        auto it2 = it;
        ++it2;

        if (it2 != flat.end()) {
            if (it->x == it2->x && it->y == it2->y &&
                (duplicates.size() == 0 || duplicates.back().x != it->x || duplicates.back().y != it->y)) {
                duplicates.push_back(*it);
            }
        }
    }

    for (const auto& d : duplicates) {
        bool first = true;

        for (auto ch = chs.begin(); ch != chs.end(); ++ch) {
            auto it = ch->begin();
            while (it != ch->end()) {
                if (it->x == d.x && it->y == d.y) {
                    if (first) {
                        first = false;
                        ++it;
                    } else {
                        ch->erase(it++);
                    }
                } else {
                    ++it;
                }
            }
        }
    }
}

bool HasDuplicates(float divisor, std::list<std::list<wxPoint>> chs)
{
    std::list<wxPoint> scaled;

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Checking for duplicates at scale %f.", divisor);

    for (const auto& ch : chs) {
        for (const auto& it : ch) {
            scaled.push_back(wxPoint((float)it.x * divisor, (float)it.y * divisor));
        }
    }

    scaled.sort(point_compare);

    for (auto it = scaled.begin(); it != scaled.end(); ++it) {
        auto it2 = it;
        ++it2;

        if (it2 != scaled.end()) {
            if (it->x == it2->x && it->y == it2->y)
                return true;
        }
    }

    return false;
}

bool CustomModel::ImportLORModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxXmlDocument doc(filename);

    if (doc.IsOk()) {
        logger_base.debug("Loading LOR model %s.", (const char*)filename.c_str());

        wxXmlNode* root = doc.GetRoot();

        std::list<std::list<wxPoint>> chs;

        for (wxXmlNode* n1 = root->GetChildren(); n1 != nullptr; n1 = n1->GetNext()) {
            if (n1->GetName() == "DrawObjects") {
                for (wxXmlNode* n2 = n1->GetChildren(); n2 != nullptr; n2 = n2->GetNext()) {
                    if (n2->GetName() == "DrawObject") {
                        for (wxXmlNode* n3 = n2->GetChildren(); n3 != nullptr; n3 = n3->GetNext()) {
                            if (n3->GetName() == "DrawPoints") {
                                std::list<wxPoint> points;
                                for (wxXmlNode* n4 = n3->GetChildren(); n4 != nullptr; n4 = n4->GetNext()) {
                                    if (n4->GetName() == "DrawPoint") {
                                        points.push_back(wxPoint(wxAtoi(n4->GetAttribute("X", "-5")) / 5, wxAtoi(n4->GetAttribute("Y", "-1")) / 5));
                                    }
                                }
                                chs.push_back(points);
                            }
                        }
                    }
                }
            }
        }

        wxFileName fn(filename);
        wxString newname = xlights->AllModels.GenerateModelName(fn.GetName().ToStdString());
        SetProperty("name", newname, true);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "CustomModel::ImportLORModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "CustomModel::ImportLORModel");

        if (chs.size() == 0) {
            logger_base.error("No model data found.");
            wxMessageBox("Unable to import model data.");
            return false;
        }

        int minx = 999999999;
        int maxx = -1;
        int miny = 999999999;
        int maxy = -1;

        for (auto ch = chs.begin(); ch != chs.end(); ++ch) {
            for (auto it = ch->begin(); it != ch->end(); ++it) {
                if (it->x >= 0) {
                    if (it->x < minx)
                        minx = it->x;
                    if (it->x > maxx)
                        maxx = it->x;
                }
                if (it->y >= 0) {
                    if (it->y < miny)
                        miny = it->y;
                    if (it->y > maxy)
                        maxy = it->y;
                }
            }
        }

        for (auto ch = chs.begin(); ch != chs.end(); ++ch) {
            for (auto it = ch->begin(); it != ch->end(); ++it) {
                it->x = (it->x - minx);
                it->y = (it->y - miny);
            }
        }

        maxx -= minx;
        maxy -= miny;

        float divisor = 0.1f;
        if (HasDuplicates(1.0, chs)) {
            DisplayWarning("This model is not going to import correctly as one or more pixels overlap.");

            RemoveDuplicatePixels(chs);
        }

        while (HasDuplicates(divisor, chs)) {
            divisor += 0.1f;

            if (divisor >= 1.0f)
                break;
        }

        divisor -= 0.1f + 0.01f;

        while (HasDuplicates(divisor, chs)) {
            divisor += 0.01f;

            if (divisor >= 1.0f)
                break;
        }

        maxx = ((float)maxx * divisor) + 1;
        maxy = ((float)maxy * divisor) + 1;

        logger_base.debug("Divisor chosen %f. Model dimensions %d,%d", divisor, maxx + 1, maxy + 1);

        SetProperty("parm1", wxString::Format("%i", maxx));
        SetProperty("parm2", wxString::Format("%i", maxy));

        int* data = (int*)malloc(maxx * maxy * sizeof(int));
        memset(data, 0x00, maxx * maxy * sizeof(int));

        int c = 1;

        for (auto ch = chs.begin(); ch != chs.end(); ++ch) {
            for (auto it = ch->begin(); it != ch->end(); ++it) {
                int x = (float)it->x * divisor;
                int y = (float)it->y * divisor;

                wxASSERT(x >= 0 && x < maxx);
                wxASSERT(y >= 0 && y < maxy);

                data[y * maxx + x] = c;
            }
            c++;
        }

        std::string cm = "";
        for (int y = 0; y < maxy; ++y) {
            for (int x = 0; x < maxx; ++x) {
                if (data[y * maxx + x] != 0) {
                    cm += wxString::Format("%i", data[y * maxx + x]);
                }
                if (x != maxx - 1)
                    cm += ",";
            }

            if (y != maxy - 1)
                cm += ";";
        }
        free(data);

        SetProperty("CustomModel", cm);
        logger_base.debug("Model import done.");
        return true;
    } else {
        DisplayError("Failure loading LOR model file.");
        return false;
    }
}

// This is required because users dont need to have their start nodes for each string in ascending
// order ... this helps us name the strings correctly
int CustomModel::MapPhysicalStringToLogicalString(int string) const
{
    if (_strings == 1)
        return string;

    // FIXME
    // This is not very efficient ... n^2 algorithm ... but given most people will have a small
    // number of strings and it is super simple and only used on controller upload i am hoping
    // to get away with it

    std::vector<int> stringOrder;
    for (int curr = 0; curr < _strings; curr++) {
        int count = 0;
        for (int s = 0; s < _strings; s++) {
            if (stringStartChan[s] < stringStartChan[curr] && s != curr) {
                count++;
            }
        }
        stringOrder.push_back(count);
    }
    return stringOrder[string];
}

int CustomModel::GetNumPhysicalStrings() const
{
    int ts = GetSmartTs();
    if (ts <= 1) {
        return _strings;
    } else {
        int strings = _strings / ts;
        if (strings == 0)
            strings = 1;
        return strings;
    }
}

bool CustomModel::ChangeStringCount(long count, std::string& message)
{
    if (count == _strings) {
        return true;
    }

    ModelXml->DeleteAttribute("CustomStrings");
    ModelXml->AddAttribute("CustomStrings", wxString::Format("%d", count));

    if (count != 1) {    
        _strings = count;
        if (_hasIndivNodes) {
            for (int x = 0; x < count; x++) {
                _indivStartNodes[x] = ComputeStringStartNode(x);
            }
        }
    }

    AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MatrixModel::ChangeStringCount::MatrixStringCount");
    AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MatrixModel::ChangeStringCount::MatrixStringCount");
    return true;
}
