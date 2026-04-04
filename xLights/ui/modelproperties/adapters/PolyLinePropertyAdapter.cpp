/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "PolyLinePropertyAdapter.h"
#include "../PropertyGridHelpers.h"
#include "../../../models/PolyLineModel.h"
#include "../../../models/Model.h"
#include "../../../models/OutputModelManager.h"
#include "UtilFunctions.h"

static const char* POLY_CORNER_VALUES[] = {
    "Leading Segment",
    "Trailing Segment",
    "Neither"
};
static wxPGChoices POLY_CORNERS(wxArrayString(3, POLY_CORNER_VALUES));

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

PolyLinePropertyAdapter::PolyLinePropertyAdapter(Model& model)
    : ModelPropertyAdapter(model), _polyLine(static_cast<PolyLineModel&>(model)) {}

void PolyLinePropertyAdapter::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) {
    wxPGProperty* p;
    if (_polyLine.IsSingleNode()) {
        p = grid->Append(new wxUIntProperty("# Lights", "PolyLineNodes", _polyLine.GetTotalLightCount()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("# Nodes", "PolyLineNodes", _polyLine.GetTotalLightCount()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");

        p = grid->Append(new wxUIntProperty("Lights/Node", "PolyLineLights", _polyLine.GetLightsPerNode()));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 300);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxUIntProperty("Strings", "PolyLineStrings", _polyLine.GetNumStrings()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 48);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_polyLine.GetNumStrings() == 1) {
        // cant set start node
    } else {
        p = grid->Append(new wxStringProperty("Start Nodes", "ModelIndividualStartNodes", ""));

        std::string nm = Model::StartChanAttrName(0);
        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, _polyLine.GetIndivStartNode(0)));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)_polyLine.GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (_polyLine.HasIndivStartNodes()) {
            int c = _polyLine.GetNumStrings();
            int nodeCount = (int)_polyLine.GetNodeCount();
            for (int x = 0; x < c; x++) {
                int v = _polyLine.GetIndivStartNode(x);
                if (v < 1) v = 1;
                if (v > nodeCount) v = nodeCount;
                if (x == 0) {
                    psn->SetValue(v);
                } else {
                    nm = Model::StartChanAttrName(x);
                    wxPGProperty* pChild = grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                    pChild->SetAttribute("Min", 1);
                    pChild->SetAttribute("Max", nodeCount);
                    pChild->SetEditor("SpinCtrl");
                }
            }
        } else {
            psn->Enable(false);
        }
    }

    grid->Append(new wxEnumProperty("Starting Location", "PolyLineStart", LEFT_RIGHT, _polyLine.GetIsLtoR() ? 0 : 1));

    grid->Append(new wxDropPatternProperty("Drop Pattern", "IciclesDrops", _polyLine.GetDropPattern()));

    p = grid->Append(new wxBoolProperty("Alternate Drop Nodes", "AlternateNodes", _polyLine.HasAlternateNodes()));
    p->SetEditor("CheckBox");

    p = grid->Append(new wxFloatProperty("Height", "ModelHeight", _polyLine.GetModelHeight()));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");

    int numSegments = _polyLine.GetNumSegments();
    auto segSizes = _polyLine.GetSegmentsSizes();
    auto corners = _polyLine.GetCorners();

    p = grid->Append(new wxStringProperty("Segments", "ModelIndividualSegments", ""));
    for (int x = 0; x < numSegments; x++) {
        wxString nm = wxString::Format("Segment %d", x + 1);
        grid->AppendIn(p, new wxUIntProperty(nm, _polyLine.SegAttrName(x), segSizes[x]));
    }
    if (_polyLine.AreSegsExpanded()) grid->Collapse(p);

    p = grid->Append(new wxStringProperty("Corner Settings", "PolyCornerProperties", ""));
    for (int x = 0; x < numSegments + 1; x++) {
        wxString nm = wxString::Format("Corner %d", x + 1);
        grid->AppendIn(p, new wxEnumProperty(nm, _polyLine.CornerAttrName(x), POLY_CORNERS, corners[x] == "Leading Segment" ? 0 : corners[x] == "Trailing Segment" ? 1 : 2));
    }
    if (_polyLine.AreSegsExpanded()) grid->Collapse(p);
}

int PolyLinePropertyAdapter::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if ("PolyLineNodes" == event.GetPropertyName()) {
        _polyLine.SetTotalLightCount((int)event.GetPropertyValue().GetLong());
        wxPGProperty* sp = grid->GetPropertyByLabel("# Nodes");
        if (sp == nullptr) {
            sp = grid->GetPropertyByLabel("# Lights");
        }
        sp->SetValueFromInt(_polyLine.GetTotalLightCount());
        _polyLine.SetAutoDistribute(true);
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyLineNodes");
        return 0;
    } else if ("PolyLineLights" == event.GetPropertyName()) {
        _polyLine.SetLightsPerNode((int)event.GetPropertyValue().GetLong());
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "PolyLineModel::OnPropertyGridChange::PolyLineLights");
        return 0;
    } else if ("PolyLineStart" == event.GetPropertyName()) {
        _polyLine.SetDirection(event.GetValue().GetLong() == 0 ? "L" : "R");
        _polyLine.SetIsLtoR(event.GetValue().GetLong() == 0);
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "PolyLineModel::OnPropertyGridChange::PolyLineStart");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        std::string segment = event.GetPropertyName().ToStdString();
        int idx = ExtractTrailingInt(segment) - 1;
        _polyLine.SetRawSegmentSize(idx, event.GetPropertyValue().GetLong());
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_RELOAD_MODELLIST, "PolyLineModel::OnPropertyGridChange::ModelIndividualSegments2");
        return 0;
    } else if (event.GetPropertyName().StartsWith("PolyCornerProperties.")) {
        std::string corner = event.GetPropertyName().ToStdString();
        int x = ExtractTrailingInt(corner) - 1;
        int numSegments = _polyLine.GetNumSegments();
        _polyLine.SetCornerString(x, POLY_CORNER_VALUES[event.GetPropertyValue().GetLong()]);
        std::string cornerVal = POLY_CORNER_VALUES[event.GetPropertyValue().GetLong()];
        if (x == 0) {
            _polyLine.SetLeadOffset(x, cornerVal == "Leading Segment" ? 1.0 : cornerVal == "Trailing Segment" ? 0.0 : 0.5);
        } else if (x == numSegments) {
            _polyLine.SetTrailOffset(x - 1, cornerVal == "Leading Segment" ? 0.0 : cornerVal == "Trailing Segment" ? 1.0 : 0.5);
        } else {
            _polyLine.SetTrailOffset(x - 1, cornerVal == "Leading Segment" ? 0.0 : cornerVal == "Trailing Segment" ? 1.0 : 0.5);
            _polyLine.SetLeadOffset(x, cornerVal == "Leading Segment" ? 1.0 : cornerVal == "Trailing Segment" ? 0.0 : 0.5);
        }
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyCornerProperties");
        return 0;
    } else if ("PolyLineStrings" == event.GetPropertyName()) {
        int old_string_count = _polyLine.GetNumStrings();
        int new_string_count = event.GetValue().GetInteger();
        _polyLine.SetNumStrings(new_string_count);
        _polyLine.SetHasIndivStartNodes(new_string_count > 1);
        if (old_string_count != new_string_count) {
            if (_polyLine.HasIndivStartNodes()) {
                _polyLine.SetIndivStartNodesCount(new_string_count);
                for (int x = 0; x < new_string_count; x++) {
                    _polyLine.SetIndivStartNode(x, _polyLine.ComputeStringStartNode(x));
                }
            }
        }
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::PolyLineStrings");
        return 0;
    } else if (event.GetPropertyName() == "ModelIndividualStartChannels") {
        bool hasIndivChans = event.GetValue().GetBool();
        _polyLine.SetHasIndividualStartChannels(hasIndivChans);
        if (hasIndivChans && _polyLine.HasIndivStartNodes() && _polyLine.GetIndivStartNodesCount() > 0) {
            int c = _polyLine.GetNumStrings();
            _polyLine.SetIndivStartChannelCount(c);
            int32_t modelStartChannel = _polyLine.GetNumberFromChannelString(_polyLine.GetModelStartChannel());
            int chanPerNode = _polyLine.GetNodeChannelCount(_polyLine.GetStringType());
            for (int x = 0; x < c; ++x) {
                if (x == 0) {
                    _polyLine.SetIndividualStartChannel(x, _polyLine.GetModelStartChannel());
                } else {
                    int node = _polyLine.GetIndivStartNode(x);
                    int32_t startChannel = modelStartChannel + (node - 1) * chanPerNode;
                    _polyLine.SetIndividualStartChannel(x, std::to_string(startChannel));
                }
            }
        } else if (hasIndivChans) {
            int c = _polyLine.GetNumStrings();
            _polyLine.SetIndivStartChannelCount(c);
            for (int x = 0; x < c; ++x) {
                _polyLine.SetIndividualStartChannel(x, _polyLine.ComputeStringStartChannel(x));
            }
        } else {
            _polyLine.SetIndivStartChannelCount(0);
        }
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartChannels");
        return 0;
    } else if (event.GetPropertyName() == "ModelIndividualStartNodes") {
        _polyLine.SetHasIndivStartNodes(event.GetValue().GetBool());
        if (_polyLine.HasIndivStartNodes()) {
            int strings = _polyLine.GetNumStrings();
            _polyLine.SetIndivStartNodesCount(strings);
            for (int x = 0; x < strings; x++) {
                _polyLine.SetIndivStartNode(x, _polyLine.ComputeStringStartNode(x));
            }
        }
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes");
        return 0;
    } else if (event.GetPropertyName().StartsWith("ModelIndividualStartNodes.String")) {
        wxString s = event.GetPropertyName().substr(strlen("ModelIndividualStartNodes.String"));
        int string = wxAtoi(s) - 1;
        int value = event.GetValue().GetInteger();
        if (value < 1) value = 1;
        if (value > (int)_polyLine.GetNodeCount()) value = (int)_polyLine.GetNodeCount();
        _polyLine.SetIndivStartNode(string, value);
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS, "PolyLineModel::OnPropertyGridChange::ModelIndividualStartNodes2");
        return 0;
    } else if ("IciclesDrops" == event.GetPropertyName()) {
        _polyLine.SetDropPattern(event.GetPropertyValue().GetString().ToStdString());
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE |
                    OutputModelManager::WORK_RELOAD_MODELLIST |
                    OutputModelManager::WORK_CALCULATE_START_CHANNELS |
                    OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS |
                    OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyLineModel::OnPropertyGridChange::IciclesDrops");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        _polyLine.SetAlternateNodes(event.GetPropertyValue().GetBool());
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "PolyLineModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    } else if (!_polyLine.GetModelScreenLocation().IsLocked() && !_polyLine.IsFromBase() && "ModelHeight" == event.GetPropertyName()) {
        float height = event.GetValue().GetDouble();
        if (std::abs(height) < 0.01f) {
            height = height < 0.0f ? -0.01f : 0.01f;
        }
        _polyLine.SetModelHeight(height);
        _polyLine.IncrementChangeCount();
        _polyLine.AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_CHANGE, "PolyLineModel::OnPropertyGridChange::ModelHeight");
        return 0;
    } else if ((_polyLine.GetModelScreenLocation().IsLocked() || _polyLine.IsFromBase()) && "ModelHeight" == event.GetPropertyName()) {
        event.Veto();
        return 0;
    }
    return ModelPropertyAdapter::OnPropertyGridChange(grid, event);
}

int PolyLinePropertyAdapter::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    } else if (event.GetPropertyName().StartsWith("PolyCornerProperties.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(6, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    }
    return -1;
}

void PolyLinePropertyAdapter::OnPropertyGridItemCollapsed(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        _polyLine.SetSegsCollapsed(true);
    }
}

void PolyLinePropertyAdapter::OnPropertyGridItemExpanded(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "ModelIndividualSegments") {
        _polyLine.SetSegsCollapsed(false);
    }
}
