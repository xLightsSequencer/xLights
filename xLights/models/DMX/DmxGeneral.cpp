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

    screenLocation.SetRenderSize(1, 1);
}

void DmxGeneral::DrawModel(ModelPreview* preview, DrawGLUtils::xlAccumulator& va2, DrawGLUtils::xl3Accumulator& va3, const xlColor* c, float& sx, float& sy, float& sz, bool active, bool is_3d)
{
    size_t nodeCount = Nodes.size();
    DrawGLUtils::xlAccumulator& va = is_3d ? va3 : va2;

    if (red_channel > nodeCount ||
        green_channel > nodeCount ||
        blue_channel > nodeCount ||
        white_channel > nodeCount)
    {
        return;
    }

    xlColor ccolor(xlWHITE);
    xlColor black(xlBLACK);
    xlColor color;
    if (c != nullptr) {
        color = *c;
    }

    float width = ((BoxedScreenLocation)screenLocation).GetMWidth();
    float height = ((BoxedScreenLocation)screenLocation).GetMHeight();

    float bar_gap = (0.2 / (nodeCount)) * height;
    float bar_height = (0.8 / (nodeCount + 1)) * height;

    int trans = color == xlBLACK ? blackTransparency : transparency;

    if (red_channel > 0 && green_channel > 0 && blue_channel > 0) {

        xlColor proxy = xlBLACK;
        if (white_channel > 0)
        {
            Nodes[white_channel - 1]->GetColor(proxy);
        }

        if (proxy == xlBLACK)
        {
            Nodes[red_channel - 1]->GetColor(proxy);
            Nodes[green_channel - 1]->GetColor(proxy);
            Nodes[blue_channel - 1]->GetColor(proxy);
        }
    }
    else if (white_channel > 0)
    {
        xlColor proxy;
        Nodes[white_channel - 1]->GetColor(proxy);
    }

    ApplyTransparency(ccolor, trans, trans);

    // draw the bars
	xlColor proxy;
	xlColor red(xlRED);
	xlColor green(xlGREEN);
	xlColor blue(xlBLUE);
	xlColor white(xlWHITE);
	ApplyTransparency(red, trans, trans);
	ApplyTransparency(green, trans, trans);
	ApplyTransparency(blue, trans, trans);

	va.AddRect(sx - bar_gap - 2 - width / 2, sy - bar_gap - 2 - height / 2, sx + width + bar_gap + 2 - width / 2, sy + height - bar_gap + 2 - height / 2, sz, ccolor);
    va.AddRect(sx - bar_gap - width / 2, sy - bar_gap - height / 2, sx + width + bar_gap - width / 2, sy + height - bar_gap - height / 2, sz, black);

    for (int i = 1; i <= nodeCount; ++i) {
		Nodes[i - 1]->GetColor(proxy);
		float val = (float)proxy.red;
		float offsetx = val / 255.0 * width;
		if (i == red_channel) {
			proxy = red;
		}
		else if (i == green_channel) {
			proxy = green;
		}
		else if (i == blue_channel) {
			proxy = blue;
		}
		else if (i == white_channel) {
			proxy = white;
		}
		else {
			proxy = ccolor;
		}
		va.AddRect(sx - width / 2.0, sy + (bar_gap + bar_height) * (nodeCount - i + 1.0) - height / 2.0, sx + offsetx - width / 2.0, sy + (bar_gap + bar_height) * (nodeCount - i + 1.0) - bar_height - height / 2.0, sz, proxy);
	}

    va.Finish(GL_TRIANGLES);
}

void DmxGeneral::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
    if (!IsActive()) return;

    float sz = 0;
    DrawGLUtils::xl3Accumulator dummy;
    DrawModel(preview, va, dummy, c, sx, sy, sz, active, false);
}

void DmxGeneral::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
    if (!IsActive()) return;

    DrawGLUtils::xlAccumulator dummy;
    DrawModel(preview, dummy, va, c, sx, sy, sz, active, true);
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

void DmxGeneral::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
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
