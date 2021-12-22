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
#include <wx/xml/xml.h>

#include "DmxGeneral.h"
#include "../ModelScreenLocation.h"
#include "../../ModelPreview.h"
#include "../../RenderBuffer.h"
#include "../../xLightsVersion.h"
#include "../../xLightsMain.h"
#include "../../UtilFunctions.h"

DmxGeneral::DmxGeneral(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
  : DmxModel(node, manager, zeroBased)
{
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxGeneral::~DmxGeneral()
{
    //dtor
}

void DmxGeneral::AddTypeProperties(wxPropertyGridInterface *grid) {

    DmxModel::AddTypeProperties(grid);

    AddColorTypeProperties(grid);
}

int DmxGeneral::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event)
{

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxGeneral::InitModel() {
    DmxModel::InitModel();

    DisplayAs = "DmxGeneral";

    StringType = "Single Color White";
    parm2 = 1;
    parm3 = 1;

    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "0"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "0"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "0"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));

    screenLocation.SetRenderSize(1, 1, 1);
}

void DmxGeneral::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxgeneral \n");

    ExportBaseParameters(f);

    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");

    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));

    f.Write(" >\n");

    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    f.Write("</dmxgeneral>");
    f.Close();
}

void DmxGeneral::ImportXlightsModel(std::string const& filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) {
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "dmxgeneral")
        {
            ImportBaseParameters(root);

            wxString name = root->GetAttribute("name");
            wxString v = root->GetAttribute("SourceVersion");

            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString wc = root->GetAttribute("DmxWhiteChannel");
            wxString sc = root->GetAttribute("DmxShutterChannel");
            wxString so = root->GetAttribute("DmxShutterOpen");
            wxString bl = root->GetAttribute("DmxBeamLimit");
            wxString dbl = root->GetAttribute("DmxBeamLength", "1");
            wxString dbw = root->GetAttribute("DmxBeamWidth", "1");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            ImportModelChildren(root, xlights, newname);

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxGeneral::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxGeneral::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxGeneral model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxGeneral model file.");
    }
}
