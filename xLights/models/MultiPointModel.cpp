/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
//#include <wx/xml/xml.h>
//#include <wx/msgdlg.h>
//#include <wx/log.h>
//#include <wx/filedlg.h>

//#include <glm/glm.hpp>
//#include <glm/gtx/matrix_transform_2d.hpp>
//#include <glm/gtx/rotate_vector.hpp>
//#include <glm/mat3x3.hpp>

#include "MultiPointModel.h"
//#include "../support/VectorMath.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
//#include "UtilFunctions.h"
//#include "../ModelPreview.h"

//#include <log4cpp/Category.hh>

MultiPointModel::MultiPointModel(const ModelManager &manager) : ModelWithScreenLocation(manager) {
    parm1 = parm2 = parm3 = 0;
}

MultiPointModel::MultiPointModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    MultiPointModel::SetFromXml(node, zeroBased);
}

MultiPointModel::~MultiPointModel()
{
    //dtor
}

bool MultiPointModel::IsNodeFirst(int n) const
{
    return (GetIsLtoR() && n == 0) || (!GetIsLtoR() && n == Nodes.size() - 1);
}

int MultiPointModel::MapToNodeIndex(int strand, int node) const {
    return strand * parm2 + node;
}

void MultiPointModel::InitModel()
{
    _strings = wxAtoi(ModelXml->GetAttribute("MultiStrings", "1"));

    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints", "2"));

    if (num_points < 2) {
        // This is not good ... so add in a second point
        num_points = 2;
    }

    parm1 = 1;
    parm2 = num_points;
    
    InitLine();

    // read in the point data from xml
    std::vector<xlMultiPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData", "0.0, 0.0, 0.0, 0.0, 0.0, 0.0");
    wxArrayString point_array = wxSplit(point_data, ',');
    while (point_array.size() < num_points * 3) point_array.push_back("0.0");
    for (int i = 0; i < num_points; ++i) {
        pPos[i].x = wxAtof(point_array[i * 3]);
        pPos[i].y = wxAtof(point_array[i * 3 + 1]);
        pPos[i].z = wxAtof(point_array[i * 3 + 2]);
    }

    // calculate min/max for the model
    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for (int i = 0; i < num_points; ++i) {
        if (pPos[i].x < minX) minX = pPos[i].x;
        if (pPos[i].y < minY) minY = pPos[i].y;
        if (pPos[i].z < minZ) minZ = pPos[i].z;
        if (pPos[i].x > maxX) maxX = pPos[i].x;
        if (pPos[i].y > maxY) maxY = pPos[i].y;
        if (pPos[i].z > maxZ) maxZ = pPos[i].z;
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    for (int i = 0; i < num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (pPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (pPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (pPos[i].z - minZ) / deltaz;
        }
    }

    screenLocation.SetRenderSize(1.0, 1.0);

    height = wxAtof(GetModelXml()->GetAttribute("ModelHeight", "1.0"));
    double model_height = deltay;
    if (model_height < GetModelScreenLocation().GetRenderHt()) {
        model_height = GetModelScreenLocation().GetRenderHt();
    }

    // place the nodes/coords along each line segment
    int i = 0;
    if ( Nodes.size() > 0 && (BufferWi > 1 || Nodes.front()->Coords.size() > 1))
    {
        for (auto& n : Nodes)
        {
            for (auto& c : n->Coords)
            {
                c.screenX = pPos[i].x;
                c.screenY = pPos[i].y;
                c.screenZ = pPos[i].z;
            }
            i++;
        }
    }
    else if (Nodes.size() > 0)
    {
        // 1 node 1 light
        Nodes.front()->Coords.front().screenY = 0.0;
        Nodes.front()->Coords.front().screenX = 0.5;
    }
}

// initialize buffer coordinates
// parm1=Number of Strings/Arches/Canes
// parm2=Pixels Per String/Arch/Cane
void MultiPointModel::InitLine() {
    int numLights = parm1 * parm2;
    Nodes.clear();
    SetNodeCount(parm1,parm2,rgbOrder);
    SetBufferSize(1,SingleNode?parm1:numLights);
    int LastStringNum=-1;
    int chan = 0;
    int ChanIncr = GetNodeChannelCount(StringType);
    size_t NodeCount=GetNodeCount();
    if (!IsLtoR) {
        ChanIncr = -ChanIncr;
    }

    int idx = 0;
    for(size_t n=0; n<NodeCount; n++) {
        if (Nodes[n]->StringNum != LastStringNum) {
            LastStringNum=Nodes[n]->StringNum;
            chan=stringStartChan[LastStringNum];
            if (!IsLtoR) {
                chan += NodesPerString(LastStringNum) * GetNodeChannelCount(StringType);
                chan += ChanIncr;
            }
        }
        Nodes[n]->ActChan=chan;
        chan+=ChanIncr;
        Nodes[n]->Coords.resize(SingleNode?parm2:parm3);
        size_t CoordCount=GetCoordCount(n);
        for(size_t c=0; c < CoordCount; c++) {
            Nodes[n]->Coords[c].bufX=idx;
            Nodes[n]->Coords[c].bufY=0;
        }
        idx++;
    }
}
void MultiPointModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{
    wxPGProperty* p;
    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("# Lights", "MultiPointNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(false); // number of nodes is determined by number of points
    }
    else {
        p = grid->Append(new wxUIntProperty("# Nodes", "MultiPointNodes", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
        p->Enable(false); // number of nodes is determined by number of points
    }

    p = grid->Append(new wxUIntProperty("Strings", "MultiPointStrings", _strings));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 48);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (_strings == 1) {
        // cant set start node
    } else {
        wxString nm = StartNodeAttrName(0);
        bool hasIndivNodes = ModelXml->HasAttribute(nm);

        p = grid->Append(new wxBoolProperty("Indiv Start Nodes", "ModelIndividualStartNodes", hasIndivNodes));
        p->SetAttribute("UseCheckbox", true);

        wxPGProperty* psn = grid->AppendIn(p, new wxUIntProperty(nm, nm, wxAtoi(ModelXml->GetAttribute(nm, "1"))));
        psn->SetAttribute("Min", 1);
        psn->SetAttribute("Max", (int)GetNodeCount());
        psn->SetEditor("SpinCtrl");

        if (hasIndivNodes) {
            int c = _strings;
            for (int x = 0; x < c; x++) {
                nm = StartNodeAttrName(x);
                std::string val = ModelXml->GetAttribute(nm, "").ToStdString();
                if (val == "") {
                    val = ComputeStringStartNode(x);
                    ModelXml->DeleteAttribute(nm);
                    ModelXml->AddAttribute(nm, val);
                }
                int v = wxAtoi(val);
                if (v < 1)
                    v = 1;
                if (v > NodesPerString())
                    v = NodesPerString();
                if (x == 0) {
                    psn->SetValue(v);
                } else {
                    grid->AppendIn(p, new wxUIntProperty(nm, nm, v));
                }
            }
        } else {
            psn->Enable(false);
        }
    }

    p = grid->Append(new wxFloatProperty("Height", "ModelHeight", height));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
}

int MultiPointModel::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if ("MultiPointNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        wxPGProperty* sp = grid->GetPropertyByLabel("# Nodes");
        if (sp == nullptr) {
            sp = grid->GetPropertyByLabel("# Lights");
        }
        sp->SetValueFromInt((int)event.GetPropertyValue().GetLong());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "MultiPointModel::OnPropertyGridChange::MultiPointNodes");
        return 0;
    }
    else if ("MultiPointStrings" == event.GetPropertyName()) {
        int old_string_count = _strings;
        int new_string_count = event.GetValue().GetInteger();
        _strings = new_string_count;
        if (old_string_count != new_string_count) {
            wxString nm = StartNodeAttrName(0);
            bool hasIndivNodes = ModelXml->HasAttribute(nm);
            if (hasIndivNodes) {
                for (int x = 0; x < old_string_count; x++) {
                    wxString nm = StartNodeAttrName(x);
                    ModelXml->DeleteAttribute(nm);
                }
                for (int x = 0; x < new_string_count; x++) {
                    wxString nm = StartNodeAttrName(x);
                    ModelXml->AddAttribute(nm, ComputeStringStartNode(x));
                }
            }
        }
        ModelXml->DeleteAttribute("MultiStrings");
        ModelXml->AddAttribute("MultiStrings", wxString::Format("%d", _strings));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "MultiPointModel::OnPropertyGridChange::MultiPointStrings");
        return 0;
    } else if (!GetModelScreenLocation().IsLocked() && !IsFromBase() && "ModelHeight" == event.GetPropertyName()) {
        height = event.GetValue().GetDouble();
        if (std::abs(height) < 0.01f) {
            if (height < 0.0f) {
                height = -0.01f;
            }
            else {
                height = 0.01f;
            }
        }
        ModelXml->DeleteAttribute("ModelHeight");
        ModelXml->AddAttribute("ModelHeight", event.GetPropertyValue().GetString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "MultiPointModel::OnPropertyGridChange::ModelHeight");
        return 0;
    } else if ((GetModelScreenLocation().IsLocked() || IsFromBase()) && "ModelHeight" == event.GetPropertyName()) {
        event.Veto();
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

std::string MultiPointModel::ComputeStringStartNode(int x) const
{
    if (x == 0)
        return "1";

    int strings = GetNumPhysicalStrings();
    int nodes = GetNodeCount();
    float nodesPerString = (float)nodes / (float)strings;

    return wxString::Format("%d", (int)(x * nodesPerString + 1)).ToStdString();
}

int MultiPointModel::NodesPerString() const
{
    return Model::NodesPerString();
}

 int MultiPointModel::NodesPerString(int string) const
{
     int num_nodes = 0;
     if (_strings == 1) {
        return NodesPerString();
     } else {
        if (SingleNode) {
            return 1;
        } else {
            wxString nm = StartNodeAttrName(0);
            bool hasIndivNodes = ModelXml->HasAttribute(nm);
            int v1 = 0;
            int v2 = 0;
            if (hasIndivNodes) {
                nm = StartNodeAttrName(string);
                std::string val = ModelXml->GetAttribute(nm, "").ToStdString();
                v1 = wxAtoi(val);
                if (string < _strings - 1) { // not last string
                    nm = StartNodeAttrName(string + 1);
                    val = ModelXml->GetAttribute(nm, "").ToStdString();
                    v2 = wxAtoi(val);
                }
            } else {
                v1 = wxAtoi(ComputeStringStartNode(string));
                if (string < _strings - 1) { // not last string
                    v2 = wxAtoi(ComputeStringStartNode(string + 1));
                }
            }
            if (string < _strings - 1) { // not last string
                num_nodes = v2 - v1;
            } else {
                num_nodes = GetNodeCount() - v1 + 1;
            }
        }
        int ts = GetSmartTs();
        if (ts <= 1) {
            return num_nodes;
        } else {
            return num_nodes * ts;
        }
     }
}

int MultiPointModel::OnPropertyGridSelection(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{
    if (event.GetPropertyName().StartsWith("ModelIndividualSegments.")) {
        wxString str = event.GetPropertyName();
        str = str.SubString(str.Find(".") + 1, str.length());
        str = str.SubString(3, str.length());
        int segment = wxAtoi(str) - 1;
        return segment;
    }
    return -1;
}

// This is required because users dont need to have their start nodes for each string in ascending
// order ... this helps us name the strings correctly
int MultiPointModel::MapPhysicalStringToLogicalString(int string) const
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

int MultiPointModel::GetNumPhysicalStrings() const
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

void MultiPointModel::DeleteHandle(int handle_) {
    // handle is offset by 1 due to the center handle at 0
    int handle = handle_ - 1;
    GetModelScreenLocation().DeleteHandle(handle);
}

void MultiPointModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "multipointmodel") {
        wxString name = root->GetAttribute("name");
        wxString p1 = root->GetAttribute("parm1");
        wxString p2 = root->GetAttribute("parm2");
        wxString p3 = root->GetAttribute("parm3");
        wxString st = root->GetAttribute("StringType");
        wxString ps = root->GetAttribute("PixelSize");
        wxString t = root->GetAttribute("Transparency", "0");
        wxString mb = root->GetAttribute("ModelBrightness", "0");
        wxString a = root->GetAttribute("Antialias");
        wxString ss = root->GetAttribute("StartSide");
        wxString dir = root->GetAttribute("Dir");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        wxString v = root->GetAttribute("SourceVersion");
        wxString pts = root->GetAttribute("NumPoints");
        wxString point_data = root->GetAttribute("PointData");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model

        SetProperty("parm1", p1);
        SetProperty("parm2", p2);
        SetProperty("parm3", p3);
        SetProperty("StringType", st);
        SetProperty("PixelSize", ps);
        SetProperty("Transparency", t);
        SetProperty("ModelBrightness", mb);
        SetProperty("Antialias", a);
        SetProperty("StartSide", ss);
        SetProperty("Dir", dir);
        SetProperty("StrandNames", sn);
        SetProperty("NodeNames", nn);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);
        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        ModelXml->DeleteAttribute("NumPoints");
        ModelXml->AddAttribute("NumPoints", pts);

        ModelXml->DeleteAttribute("PointData");
        ModelXml->AddAttribute("PointData", point_data);

        GetModelScreenLocation().Read(ModelXml);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "MultiPointModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "MultiPointModel::ImportXlightsModel");
    } else {
        DisplayError("Failure loading MultiPoint model file.");
    }
}

void MultiPointModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty())
        return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString pts = ModelXml->GetAttribute("NumPoints");
    NormalizePointData();
    wxString point_data = ModelXml->GetAttribute("PointData");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<multipointmodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StartSide=\"%s\" ", ss));
    f.Write(wxString::Format("Dir=\"%s\" ", dir));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("NumPoints=\"%s\" ", pts));
    f.Write(wxString::Format("PointData=\"%s\" ", point_data));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    //ExportDimensions(f);
    f.Write("</multipointmodel>");
    f.Close();
}

void MultiPointModel::NormalizePointData()
{
    // read in the point data from xml
    int num_points = wxAtoi(ModelXml->GetAttribute("NumPoints"));
    if (num_points < 2)
        num_points = 2;
    std::vector<xlMultiPoint> pPos(num_points);
    wxString point_data = ModelXml->GetAttribute("PointData");
    wxArrayString point_array = wxSplit(point_data, ',');
    while (point_array.size() < num_points * 3)
        point_array.push_back("0.0");
    for (int i = 0; i < num_points; ++i) {
        pPos[i].x = wxAtof(point_array[i * 3]);
        pPos[i].y = wxAtof(point_array[i * 3 + 1]);
        pPos[i].z = wxAtof(point_array[i * 3 + 2]);
    }

    float minX = 100000.0f;
    float minY = 100000.0f;
    float minZ = 100000.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;
    float maxZ = 0.0f;

    for (int i = 0; i < num_points; ++i) {
        if (pPos[i].x < minX)
            minX = pPos[i].x;
        if (pPos[i].y < minY)
            minY = pPos[i].y;
        if (pPos[i].z < minZ)
            minZ = pPos[i].z;
        if (pPos[i].x > maxX)
            maxX = pPos[i].x;
        if (pPos[i].y > maxY)
            maxY = pPos[i].y;
        if (pPos[i].z > maxZ)
            maxZ = pPos[i].z;
    }
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;

    // normalize all the point data
    for (int i = 0; i < num_points; ++i) {
        if (deltax == 0.0f) {
            pPos[i].x = 0.0f;
        } else {
            pPos[i].x = (pPos[i].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            pPos[i].y = 0.0f;
        } else {
            pPos[i].y = (pPos[i].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            pPos[i].z = 0.0f;
        } else {
            pPos[i].z = (pPos[i].z - minZ) / deltaz;
        }
    }

    ModelXml->DeleteAttribute("PointData");
    point_data = "";
    for (int i = 0; i < num_points; ++i) {
        point_data += wxString::Format("%f,", pPos[i].x);
        point_data += wxString::Format("%f,", pPos[i].y);
        point_data += wxString::Format("%f", pPos[i].z);
        if (i != num_points - 1) {
            point_data += ",";
        }
    }
    ModelXml->AddAttribute("PointData", point_data);
}
