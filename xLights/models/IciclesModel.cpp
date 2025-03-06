/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "IciclesModel.h"
#include "../OutputModelManager.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"

IciclesModel::IciclesModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager)
{
    screenLocation.SetMHeight(-0.5);
    screenLocation.SetSupportsShear(true);
    SetFromXml(node, zeroBased);
}

IciclesModel::~IciclesModel()
{
}

void IciclesModel::InitModel()
{
    wxString dropPattern = GetModelXml()->GetAttribute("DropPattern", "3,4,5,4");
    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    wxArrayString pat = wxSplit(dropPattern, ',');
    int numStrings = parm1;
    int lightsPerString = parm2;

    SetNodeCount(numStrings, lightsPerString, rgbOrder);

    std::vector<size_t> dropSizes;
    size_t maxH = 0;
    for (int x = 0; x < pat.size(); x++) {
        int d = wxAtoi(pat[x]);
        if (d >= 0) { // we dont handle drops of less than zero
            dropSizes.push_back(wxAtoi(pat[x]));
            maxH = std::max(maxH, (size_t)d);
        }
    }
    if (dropSizes.size() == 0) {
        dropSizes.push_back(5);
    }

    int width = -1;
    size_t curNode = 0;
    size_t curCoord = 0;
    for (size_t x = 0; x < numStrings; x++) {
        int lights = lightsPerString;
        size_t y = 0;
        size_t curDrop = 0;
        size_t nodesInDrop = dropSizes[curDrop];
        width++;
        while (lights > 0) {
            if (curCoord >= Nodes[curNode]->Coords.size()) {
                curNode++;
                curCoord = 0;
            }
            while (y >= dropSizes[curDrop]) {
                width++;
                y = 0;
                curDrop++;
                if (curDrop >= dropSizes.size()) {
                    curDrop = 0;
                }
                nodesInDrop = dropSizes[curDrop];
            }
            Nodes[curNode]->ActChan = stringStartChan[0] + curNode * GetNodeChannelCount(StringType);
            Nodes[curNode]->StringNum = x;
            Nodes[curNode]->Coords[curCoord].bufX = width;
            if (_alternateNodes) {
                if (y + 1 <= (nodesInDrop + 1) / 2) {
                    Nodes[curNode]->Coords[curCoord].bufY = maxH - 1 - (2 * y);
                    Nodes[curNode]->Coords[curCoord].screenY = (2 * y);
                }
                else {
                    Nodes[curNode]->Coords[curCoord].bufY = maxH - 1 - ((nodesInDrop - (y + 1)) * 2 + 1);
                    Nodes[curNode]->Coords[curCoord].screenY = ((nodesInDrop - (y + 1)) * 2 + 1);
                }
            }
            else {
                Nodes[curNode]->Coords[curCoord].bufY = maxH - y - 1;
                Nodes[curNode]->Coords[curCoord].screenY = y;
            }
            Nodes[curNode]->Coords[curCoord].screenX = width;
            lights--;
            y++;
            curCoord++;
        }
    }
    if (!IsLtoR) {
        for (size_t n = 0; n < Nodes.size(); n++) {
            for (auto& cd : Nodes[n]->Coords) {
                cd.bufX = width - cd.bufX;
                cd.screenX = width - cd.screenX;
            }
        }
    }
    SetBufferSize(maxH, width + 1);

    //single icicle move to the center
    if (width == 0) {
        for (auto& n : Nodes) {
            for (auto& c : n->Coords) {
                c.screenX = 0.5;
            }
        }
        width++;
    }
    screenLocation.SetRenderSize(width, maxH);
}

static const char* LEFT_RIGHT_VALUES[] = { "Green Square", "Blue Square" };
static wxPGChoices LEFT_RIGHT(wxArrayString(2, LEFT_RIGHT_VALUES));

void IciclesModel::AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager)
{

    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "IciclesStrings", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
    p->SetHelpString("This is typically the number of connections from the prop to your controller.");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "IciclesLights", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "IciclesLights", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 2000);
        p->SetEditor("SpinCtrl");
        p->SetHelpString("This is typically the total number of pixels per #String.");
    }

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");

    grid->Append(new wxDropPatternProperty("Drop Pattern", "IciclesDrops", GetModelXml()->GetAttribute("DropPattern", "3,4,5,4")));

    grid->Append(new wxEnumProperty("Starting Location", "IciclesStart", LEFT_RIGHT, IsLtoR ? 0 : 1));
}

int IciclesModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("IciclesStrings" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesLights" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "IciclesModel::OnPropertyGridChange::IciclesStrings");
        return 0;
    } else if ("IciclesDrops" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DropPattern");
        ModelXml->AddAttribute("DropPattern", event.GetPropertyValue().GetString());
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesDrops");
        return 0;
    } else if ("IciclesStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 ? "L" : "R");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::IciclesStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::IciclesStart");
        return 0;
    } else if ("AlternateNodes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("AlternateNodes");
        ModelXml->AddAttribute("AlternateNodes", event.GetPropertyValue().GetBool() ? "true" : "false");
        IncrementChangeCount();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "IciclesModel::OnPropertyGridChange::AlternateNodes");
        return 0;
    }
    return Model::OnPropertyGridChange(grid, event);
}

std::string IciclesModel::GetDimension() const
{
    // the height does not make sense for icicles
    return static_cast<TwoPointScreenLocation>(screenLocation).GetDimension(1.0);
}

void IciclesModel::AddDimensionProperties(wxPropertyGridInterface* grid)
{
    // the height does not make sense for icicles
    static_cast<TwoPointScreenLocation>(screenLocation).AddDimensionProperties(grid, 1.0);
}

void IciclesModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "xLights Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);

    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("DropPattern");
    wxString an = ModelXml->GetAttribute("AlternateNodes");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString s0 = ModelXml->GetAttribute("Strings");
    wxString s1 = ModelXml->GetAttribute("Start");
    wxString s2 = ModelXml->GetAttribute("Style");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<iciclemodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("DropPattern=\"%s\" ", p3));
    f.Write(wxString::Format("AlternateNodes=\"%s\" ", an));
    f.Write(wxString::Format("DisplayAs=\"%s\" ", da));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StartSide=\"%s\" ", ss));
    f.Write(wxString::Format("Dir=\"%s\" ", dir));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(wxString::Format("Strings=\"%s\" ", s0));
    f.Write(wxString::Format("Start=\"%s\" ", s1));
    f.Write(wxString::Format("Style=\"%s\" ", s2));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString aliases = SerialiseAliases();
    if (aliases != "") {
        f.Write(aliases);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString face = SerialiseFace();
    if (face != "")
    {
        f.Write(face);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    ExportDimensions(f);
    f.Write("</iciclemodel>");
    f.Close();
}

bool IciclesModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "iciclemodel") {
        wxString name = root->GetAttribute("name");
        wxString p1 = root->GetAttribute("parm1");
        wxString p2 = root->GetAttribute("parm2");
        wxString p3 = root->GetAttribute("DropPattern");
        wxString an = root->GetAttribute("AlternateNodes");
        wxString st = root->GetAttribute("StringType");
        wxString ps = root->GetAttribute("PixelSize");
        wxString t = root->GetAttribute("Transparency", "0");
        wxString mb = root->GetAttribute("ModelBrightness", "0");
        wxString a = root->GetAttribute("Antialias");
        wxString ss = root->GetAttribute("StartSide");
        wxString dir = root->GetAttribute("Dir");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        wxString da = root->GetAttribute("DisplayAs");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");
        wxString s0 = root->GetAttribute("Strings");
        wxString s1 = root->GetAttribute("Start");
        wxString s2 = root->GetAttribute("Style");

        SetProperty("parm1", p1);
        SetProperty("parm2", p2);
        SetProperty("DropPattern", p3);
        SetProperty("AlternateNodes", an);
        SetProperty("StringType", st);
        SetProperty("PixelSize", ps);
        SetProperty("Transparency", t);
        SetProperty("ModelBrightness", mb);
        SetProperty("Antialias", a);
        SetProperty("StartSide", ss);
        SetProperty("Dir", dir);
        SetProperty("StrandNames", sn);
        SetProperty("NodeNames", nn);
        SetProperty("DisplayAs", da);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);
        SetProperty("Strings", s0);
        SetProperty("Start", s1);
        SetProperty("Style", s2);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "IcicleModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "IcicleModel::ImportXlightsModel");

        return true;
    } else {
        DisplayError("Failure loading Icicle model file.");
        return false;
    }
}