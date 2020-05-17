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

#include "DmxFloodlight.h"
#include "../../ModelPreview.h"
#include "../../UtilFunctions.h"
#include "../../xLightsMain.h"
#include "../../xLightsVersion.h"

DmxFloodlight::DmxFloodlight(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased), beam_length(1)
{
    color_ability = this;
    SetFromXml(node, zeroBased);
}

DmxFloodlight::~DmxFloodlight()
{
    //dtor
}

void DmxFloodlight::AddTypeProperties(wxPropertyGridInterface* grid) {

    DmxModel::AddTypeProperties(grid);
    AddColorTypeProperties(grid);
    AddShutterTypeProperties(grid);

    auto p = grid->Append(new wxFloatProperty("Beam Display Length", "DmxBeamLength", beam_length));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
}

void DmxFloodlight::DisableUnusedProperties(wxPropertyGridInterface* grid)
{
    // disable these because the size of the object is determined by the size of the bounding box
    wxPGProperty* p = grid->GetPropertyByName("ModelPixelSize");
    if (p != nullptr) {
        p->Enable(false);
    }

    p = grid->GetPropertyByName("ModelPixelStyle");
    if (p != nullptr) {
        p->Enable(false);
    }

    DmxModel::DisableUnusedProperties(grid);
}

int DmxFloodlight::OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) {

    if ("DmxBeamLength" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("DmxBeamLength");
        ModelXml->AddAttribute("DmxBeamLength", wxString::Format("%6.4f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxFloodlight::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "DmxFloodlight::OnPropertyGridChange::DMXBeamLength");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "DmxFloodlight::OnPropertyGridChange::DMXBeamLength");
        return 0;
    }

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    if (OnShutterPropertyGridChange(grid, event, ModelXml, this) == 0) {
        return 0;
    }

    return DmxModel::OnPropertyGridChange(grid, event);
}

void DmxFloodlight::InitModel() {
    DmxModel::InitModel();
    DisplayAs = "DmxFloodlight";
    red_channel = wxAtoi(ModelXml->GetAttribute("DmxRedChannel", "1"));
    green_channel = wxAtoi(ModelXml->GetAttribute("DmxGreenChannel", "2"));
    blue_channel = wxAtoi(ModelXml->GetAttribute("DmxBlueChannel", "3"));
    white_channel = wxAtoi(ModelXml->GetAttribute("DmxWhiteChannel", "0"));
    shutter_channel = wxAtoi(ModelXml->GetAttribute("DmxShutterChannel", "0"));
    shutter_threshold = wxAtoi(ModelXml->GetAttribute("DmxShutterOpen", "1"));
    beam_length = wxAtof(ModelXml->GetAttribute("DmxBeamLength", "1.0"));
    screenLocation.SetRenderSize(1, 1);
}

void DmxFloodlight::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    if (!IsActive()) return;

    // determine if shutter is open for floods that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && shutter_channel <= Nodes.size() && active) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        }
        else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }

    size_t NodeCount=Nodes.size();

    if( red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount ||
        white_channel > NodeCount )
    {
        return;
    }

	xlColor color;
	xlColor ecolor;
	xlColor beam_color(xlWHITE);
	if (c != nullptr) {
		color = *c;
		ecolor = *c;
	}

	int trans = color == xlBLACK ? blackTransparency : transparency;
	if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {

        xlColor proxy = xlBLACK;
        if (white_channel > 0)
        {
            Nodes[white_channel - 1]->GetColor(proxy);
            beam_color = proxy;
        }

        if (proxy == xlBLACK)
        {
            Nodes[red_channel - 1]->GetColor(proxy);
            beam_color.red = proxy.red;
            Nodes[green_channel - 1]->GetColor(proxy);
            beam_color.green = proxy.red;
            Nodes[blue_channel - 1]->GetColor(proxy);
            beam_color.blue = proxy.red;
        }
	}
    else if (white_channel > 0)
    {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
        beam_color.red = proxy.red;
        beam_color.green = proxy.red;
        beam_color.blue = proxy.red;
    }

	if (!active) {
		beam_color = color;
	}

	ApplyTransparency(beam_color, trans, trans);
    if (pixelStyle == 2) {
        ecolor = beam_color;
    } else {
        ecolor.alpha = 0;
    }

    float rh = ((BoxedScreenLocation)screenLocation).GetMWidth();
    float rw = ((BoxedScreenLocation)screenLocation).GetMHeight();
	float min_size = (float)(std::min(rh, rw));

    if (shutter_open) {
        va.AddTrianglesCircle(sx, sy, min_size / 2.0f, beam_color, ecolor);
        va.Finish(GL_TRIANGLES);
    }
}

void DmxFloodlight::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
    if (!IsActive()) return;

    // determine if shutter is open for floods that support it
    bool shutter_open = true;
    if (shutter_channel > 0 && active) {
        xlColor proxy;
        Nodes[shutter_channel - 1]->GetColor(proxy);
        int shutter_value = proxy.red;
        if (shutter_value >= 0) {
            shutter_open = shutter_value >= shutter_threshold;
        }
        else {
            shutter_open = shutter_value <= std::abs(shutter_threshold);
        }
    }

    size_t NodeCount = Nodes.size();

    if (red_channel > NodeCount ||
        green_channel > NodeCount ||
        blue_channel > NodeCount)
    {
        return;
    }

    xlColor color;
    xlColor ecolor;
    xlColor beam_color(xlWHITE);
    if (c != nullptr) {
        color = *c;
        ecolor = *c;
    }

    int trans = color == xlBLACK ? blackTransparency : transparency;
    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {
        xlColor proxy;
        Nodes[red_channel - 1]->GetColor(proxy);
        beam_color.red = proxy.red;
        Nodes[green_channel - 1]->GetColor(proxy);
        beam_color.green = proxy.red;
        Nodes[blue_channel - 1]->GetColor(proxy);
        beam_color.blue = proxy.red;
    }
    if (!active) {
        beam_color = color;
    }

    ApplyTransparency(beam_color, trans, trans);
    if (pixelStyle == 2) {
        ecolor = beam_color;
    } else {
        ecolor.alpha = 0;
    }


    float rh = ((BoxedScreenLocation)screenLocation).GetMWidth();
    float rw = ((BoxedScreenLocation)screenLocation).GetMHeight();
    float min_size = (float)(std::min(rh, rw));

    glm::quat rotation = GetModelScreenLocation().GetRotationQuat();

    if (shutter_open) {
        va.AddTrianglesRotatedCircle(sx, sy, sz, rotation, min_size / 2.0f, beam_color, ecolor, beam_length);
        va.Finish(GL_TRIANGLES);
    }
}

void DmxFloodlight::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    ExportBaseParameters(f);

    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");
    wxString dbl = ModelXml->GetAttribute("DmxBeamLength", "1");

    wxString v = xlights_version_string;

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");

    f.Write(wxString::Format("DmxRedChannel=\"%s\" ", rc));
    f.Write(wxString::Format("DmxGreenChannel=\"%s\" ", gc));
    f.Write(wxString::Format("DmxBlueChannel=\"%s\" ", bc));
    f.Write(wxString::Format("DmxWhiteChannel=\"%s\" ", wc));
    f.Write(wxString::Format("DmxBeamLength=\"%s\" ", dbl));

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
    f.Write("</dmxmodel>");
    f.Close();
}

void DmxFloodlight::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    // We have already loaded gdtf properties
    if (EndsWith(filename, "gdtf")) return;

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "dmxmodel")
        {
            ImportBaseParameters(root);

            wxString name = root->GetAttribute("name");
            wxString v = root->GetAttribute("SourceVersion");

            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString wc = root->GetAttribute("DmxWhiteChannel");
            wxString dbl = root->GetAttribute("DmxBeamLength", "1");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);
            SetProperty("DmxBeamLength", dbl);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "subModel")
                {
                    AddSubmodel(n);
                }
                else if (n->GetName() == "stateInfo")
                {
                    AddState(n);
                }
            }

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "DmxFloodlight::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "DmxFloodlight::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading DmxFloodlight model file.");
        }
    }
    else
    {
        DisplayError("Failure loading DmxFloodlight model file.");
    }
}
