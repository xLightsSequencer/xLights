#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>

#include "DmxFloodlight.h"
#include "../../ModelPreview.h"
#include "../../UtilFunctions.h"
#include "../../xLightsMain.h"
#include "../../xLightsVersion.h"

DmxFloodlight::DmxFloodlight(wxXmlNode *node, const ModelManager &manager, bool zeroBased)
    : DmxModel(node, manager, zeroBased)
{
    color_ability = this;
}
DmxFloodlight::DmxFloodlight(const ModelManager &manager)
    : DmxModel(manager)
{
    color_ability = this;
}

DmxFloodlight::~DmxFloodlight()
{
    //dtor
}

void DmxFloodlight::AddTypeProperties(wxPropertyGridInterface* grid) {

    DmxModel::AddTypeProperties(grid);
    AddColorTypeProperties(grid);
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

    if (OnColorPropertyGridChange(grid, event, ModelXml, this) == 0) {
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
}

void DmxFloodlight::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active)
{
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
    va.AddTrianglesCircle(sx, sy, min_size/2.0f, beam_color, ecolor);
    va.Finish(GL_TRIANGLES);
}

void DmxFloodlight::DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active)
{
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

    va.AddTrianglesRotatedCircle(sx, sy, sz, rotation, min_size / 2.0f, beam_color, ecolor);
    va.Finish(GL_TRIANGLES);
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
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString p3 = ModelXml->GetAttribute("parm3");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString ss = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");

    wxString rc = ModelXml->GetAttribute("DmxRedChannel", "0");
    wxString gc = ModelXml->GetAttribute("DmxGreenChannel", "0");
    wxString bc = ModelXml->GetAttribute("DmxBlueChannel", "0");
    wxString wc = ModelXml->GetAttribute("DmxWhiteChannel", "0");

    wxString v = xlights_version_string;

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<dmxmodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
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
            wxString name = root->GetAttribute("name");
            wxString p1 = root->GetAttribute("parm1");
            wxString p2 = root->GetAttribute("parm2");
            wxString p3 = root->GetAttribute("parm3");
            wxString st = root->GetAttribute("StringType");
            wxString ps = root->GetAttribute("PixelSize");
            wxString t = root->GetAttribute("Transparency");
            wxString mb = root->GetAttribute("ModelBrightness");
            wxString a = root->GetAttribute("Antialias");
            wxString ss = root->GetAttribute("StartSide");
            wxString dir = root->GetAttribute("Dir");
            wxString sn = root->GetAttribute("StrandNames");
            wxString nn = root->GetAttribute("NodeNames");
            wxString v = root->GetAttribute("SourceVersion");

            wxString rc = root->GetAttribute("DmxRedChannel");
            wxString gc = root->GetAttribute("DmxGreenChannel");
            wxString bc = root->GetAttribute("DmxBlueChannel");
            wxString wc = root->GetAttribute("DmxWhiteChannel");

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
            SetProperty("DisplayAs", "DmxFloodlight");

            SetProperty("DmxRedChannel", rc);
            SetProperty("DmxGreenChannel", gc);
            SetProperty("DmxBlueChannel", bc);
            SetProperty("DmxWhiteChannel", wc);

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
