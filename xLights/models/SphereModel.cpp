#include <wx/tokenzr.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/filedlg.h>

#include "SphereModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

SphereModel::SphereModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : MatrixModel(manager)
{
    screenLocation.SetSupportsZScaling(true);
    SetFromXml(node, zeroBased);
}

SphereModel::~SphereModel()
{
}

void SphereModel::InitModel() {
    InitVMatrix(0);
    screenLocation.SetPerspective2D(0.1f);
    SetSphereCoord();
    DisplayAs = "Sphere";
}

void SphereModel::SetSphereCoord() {

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (BufferWi < 1) return;
    if (BufferHt < 1) return; // June 27,2013. added check to not divide by zero

    double RenderHt = (double)BufferHt / 1.8;
    double RenderWi = (double)RenderHt / 1.8;

    double Hradians = toRadians(360);
    double Vradians = toRadians(180);
    double Hradius = RenderWi / 2.0;
    double Vradius = RenderHt / 2.0;

    //logger_base.debug("Buffer %d,%d Render %f,%f Radius %f,%f",
    //    BufferWi, BufferHt,
    //    (float)RenderWi, (float)RenderHt,
    //    (float)Hradius, (float)Vradius);

    double HStartAngle = Hradians / 4.0 + 0.003;
    double HAngleIncr = -Hradians / (double)BufferWi;

    //logger_base.debug("Horizontal Start %d: +%f x %d",
    //    (int)toDegrees(HStartAngle), (float)toDegrees(HAngleIncr), BufferWi);

    double VStartAngle = -Vradians;
    double VAngleIncr = Vradians / ((double)BufferHt + 1.0);
    VStartAngle += VAngleIncr;

    //logger_base.debug("Vertical Start %d: +%f x %d",
    //    (int)toDegrees(VStartAngle), (float)toDegrees(VAngleIncr), BufferHt);

    size_t NodeCount = GetNodeCount();
    for (size_t n = 0; n<NodeCount; n++) {
        size_t CoordCount = GetCoordCount(n);
        for (size_t c = 0; c < CoordCount; c++) {
            double bufferX = Nodes[n]->Coords[c].bufX;
            double bufferY = Nodes[n]->Coords[c].bufY;
            double hangle = HStartAngle + bufferX * HAngleIncr;
            double vangle = VStartAngle + bufferY * VAngleIncr;

            double sv = sin(vangle);
            Nodes[n]->Coords[c].screenX = Hradius * cos(hangle) * sv;
            Nodes[n]->Coords[c].screenZ = Hradius * sin(hangle) * sv;
            Nodes[n]->Coords[c].screenY = Vradius * cos(vangle);

            //logger_base.debug("%d: %d,%d -> hangle %d vangle %d -> %f,%f,%f",
            //    n,
            //    (int)bufferX, (int)bufferY,
            //    (int)toDegrees(hangle), (int)toDegrees(vangle),
            //    Nodes[n]->Coords[c].screenX, Nodes[n]->Coords[c].screenY, Nodes[n]->Coords[c].screenZ);
        }
    }
    screenLocation.SetRenderSize(RenderWi, RenderHt, RenderWi);
}

int SphereModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    return MatrixModel::OnPropertyGridChange(grid, event);
}

void SphereModel::AddStyleProperties(wxPropertyGridInterface *grid) {
}

void SphereModel::ExportXlightsModel()
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

    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<spheremodel \n");
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
    f.Write(" >\n");
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
    f.Write("</spheremodel>");
    f.Close();
}

void SphereModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "spheremodel")
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
            wxString da = root->GetAttribute("DisplayAs");
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
            SetProperty("DisplayAs", da);
            SetProperty("PixelCount", pc);
            SetProperty("PixelType", pt);
            SetProperty("PixelSpacing", psp);

            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            GetModelScreenLocation().Write(ModelXml);
            SetProperty("name", newname, true);

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "stateInfo")
                {
                    AddState(n);
                }
                else if (n->GetName() == "subModel")
                {
                    AddSubmodel(n);
                }
                else if (n->GetName() == "faceInfo")
                {
                    AddFace(n);
                }
            }

            xlights->MarkEffectsFileDirty(true);
        }
        else
        {
            DisplayError("Failure loading Sphere model file.");
        }
    }
    else
    {
        DisplayError("Failure loading Sphere model file.");
    }
}

void SphereModel::ExportAsCustomXModel() const {

    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    float minx = 99999;
    float miny = 99999;
    float minz = 99999;
    float maxx = -99999;
    float maxy = -99999;
    float maxz = -99999;

    for (auto& n : Nodes)
    {
        minx = std::min(minx, n->Coords[0].screenX);
        miny = std::min(miny, n->Coords[0].screenY);
        minz = std::min(minz, n->Coords[0].screenZ);
        maxx = std::max(maxx, n->Coords[0].screenX);
        maxy = std::max(maxy, n->Coords[0].screenY);
        maxz = std::max(maxz, n->Coords[0].screenZ);
    }
    float w = maxx - minx;
    float h = maxy - miny;
    float d = maxz - minz;

    std::vector<std::vector<std::vector<int>>> data;
    for (int l = 0; l < BufferWi * 2 + 1; l ++)
    {
        std::vector<std::vector<int>> layer;
        for (int  r = BufferHt * 2; r >= 0; r--)
        {
            std::vector<int> row;
            for (int c = 0; c < BufferWi * 2 + 1; c++)
            {
                row.push_back(-1);
            }
            layer.push_back(row);
        }
        data.push_back(layer);
    }

    int i = 0;
    for (auto& n: Nodes)
    {
        int xx = 2 * BufferWi * (n->Coords[0].screenX - minx) / w;
        int yy = 2 * BufferHt * (n->Coords[0].screenY - miny) / h;
        int zz = 2 * BufferWi * (n->Coords[0].screenZ - minz) / d;
        wxASSERT(xx >= 0 && xx < 2 * BufferWi + 1);
        wxASSERT(yy >= 0 && yy < 2 * BufferHt + 1);
        wxASSERT(zz >= 0 && zz < 2 * BufferWi + 1);
        wxASSERT(data[zz][yy][xx] == -1);
        data[zz][yy][xx] = i++;
    }

    wxString cm = "";
    for (auto l : data)
    {
        if (cm != "") cm += "|";
        wxString ll = "";

        for (auto r : l)
        {
            if (ll != "") ll += ";";
            wxString rr = "";

            bool first = true;
            for (auto c : r)
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    rr += ",";
                }

                if (c != -1)
                {
                    rr += wxString::Format("%d ", c);
                }
            }
            ll += rr;
        }
        cm += ll;
    }

    wxString p1 = wxString::Format("%i", 2 * BufferWi + 1);
    wxString p2 = wxString::Format("%i", 2 * BufferHt + 1);
    wxString dd = wxString::Format("%i", 2 * BufferWi + 1);
    wxString p3 = wxString::Format("%i", parm3);
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString pc = ModelXml->GetAttribute("PixelCount");
    wxString pt = ModelXml->GetAttribute("PixelType");
    wxString psp = ModelXml->GetAttribute("PixelSpacing");

    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("parm3=\"%s\" ", p3));
    f.Write(wxString::Format("Depth=\"%s\" ", dd));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    if (pc != "")
        f.Write(wxString::Format("PixelCount=\"%s\" ", pc));
    if (pt != "")
        f.Write(wxString::Format("PixelType=\"%s\" ", pt));
    if (psp != "")
        f.Write(wxString::Format("PixelSpacing=\"%s\" ", psp));
    f.Write("CustomModel=\"");
    f.Write(cm);
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(" >\n");
    wxString face = SerialiseFace();
    if (face != "")
    {
        f.Write(face);
    }
    wxString state = SerialiseState();
    if (state != "")
    {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "")
    {
        f.Write(submodel);
    }
    f.Write("</custommodel>");
    f.Close();
}