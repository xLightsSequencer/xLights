/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

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
#include "../ModelPreview.h"
#include "CustomModel.h"

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
    _startLatitude = wxAtof(ModelXml->GetAttribute("StartLatitude", "-86"));
    _endLatitude = wxAtof(ModelXml->GetAttribute("EndLatitude", "86"));
    _sphereDegrees = wxAtoi(ModelXml->GetAttribute("Degrees", "360"));

    InitVMatrix(0);
    screenLocation.SetPerspective2D(0.1f);
    SetSphereCoord();
    InitSingleChannelModel();
    DisplayAs = "Sphere";
}

void SphereModel::SetSphereCoord() {

    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (BufferWi < 1) return;
    if (BufferHt < 1) return; // June 27,2013. added check to not divide by zero

    double RenderHt = (double)BufferHt / 1.8;
    double RenderWi = (double)RenderHt / 1.8;

    double Hradians = toRadians(360);
    double Hradius = RenderWi / 2.0;
    double Vradius = RenderHt / 2.0;

    //logger_base.debug("Buffer %d,%d Render %f,%f Radius %f,%f",
    //    BufferWi, BufferHt,
    //    (float)RenderWi, (float)RenderHt,
    //    (float)Hradius, (float)Vradius);

    double remove = toRadians((360.0 - _sphereDegrees));
    double fudge = toRadians((360.0 - _sphereDegrees) / (double)BufferWi);
    double HStartAngle = Hradians / 4.0 + 0.003 - remove / 2.0;
    double HAngleIncr = (-Hradians + remove - fudge) / (double)BufferWi;

    //logger_base.debug("Horizontal Start %d: +%f x %d",
    //    (int)toDegrees(HStartAngle), (float)toDegrees(HAngleIncr), BufferWi);

    double VStartAngle = toRadians(_startLatitude-90);
    double VAngleIncr = (toRadians(-1 * _startLatitude) + toRadians(_endLatitude)) / (BufferHt-1);

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
    if (event.GetPropertyName() == "StartLatitude") {
        ModelXml->DeleteAttribute("StartLatitude");
        ModelXml->AddAttribute("StartLatitude", wxString::Format("%i", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SphereModel::OnPropertyGridChange::StartLatitude");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SphereModel::OnPropertyGridChange::StartLatitude");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SphereModel::OnPropertyGridChange::StartLatitude");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SphereModel::OnPropertyGridChange::StartLatitude");
        return 0;
    }
    else if (event.GetPropertyName() == "EndLatitude") {
        ModelXml->DeleteAttribute("EndLatitude");
        ModelXml->AddAttribute("EndLatitude", wxString::Format("%i", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SphereModel::OnPropertyGridChange::EndLatitude");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SphereModel::OnPropertyGridChange::EndLatitude");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SphereModel::OnPropertyGridChange::EndLatitude");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SphereModel::OnPropertyGridChange::EndLatitude");
        return 0;
    }
    else if (event.GetPropertyName() == "Degrees") {
        ModelXml->DeleteAttribute("Degrees");
        ModelXml->AddAttribute("Degrees", wxString::Format("%i", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SphereModel::OnPropertyGridChange::Degrees");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SphereModel::OnPropertyGridChange::Degrees");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "SphereModel::OnPropertyGridChange::Degrees");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "SphereModel::OnPropertyGridChange::Degrees");
        return 0;
    }

    return MatrixModel::OnPropertyGridChange(grid, event);
}

void SphereModel::AddStyleProperties(wxPropertyGridInterface *grid) {

    wxPGProperty* p = grid->Append(new wxIntProperty("Degrees", "Degrees", _sphereDegrees));
    p->SetAttribute("Min", "45");
    p->SetAttribute("Max", "360");
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxIntProperty("Southern Latitude", "StartLatitude", _startLatitude));
    p->SetAttribute("Min", "-89");
    p->SetAttribute("Max", "-1");
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxUIntProperty("Northern Latitude", "EndLatitude", _endLatitude));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "89");
    p->SetEditor("SpinCtrl");
}

void SphereModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    
    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }
    
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
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString sl = ModelXml->GetAttribute("StartLatitude", "-86");
    wxString el = ModelXml->GetAttribute("EndLatitude", "86");
    wxString d = ModelXml->GetAttribute("Degrees", "360");

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
    f.Write(wxString::Format("Degrees=\"%s\" ", d));
    f.Write(wxString::Format("StartLatitude=\"%s\" ", sl));
    f.Write(wxString::Format("EndLatitude=\"%s\" ", el));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString aliases = SerialiseAliases();
    if (aliases != "") {
        f.Write(aliases);
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
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    ExportDimensions(f);
    f.Write("</spheremodel>");
    f.Close();
}

bool SphereModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "spheremodel") {
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
        wxString sl = root->GetAttribute("StartLatitude", "-86");
        wxString el = root->GetAttribute("EndLatitude", "86");
        wxString d = root->GetAttribute("Degrees", "360");
        wxString sn = root->GetAttribute("StrandNames");
        wxString nn = root->GetAttribute("NodeNames");
        //wxString v = root->GetAttribute("SourceVersion");
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
        SetProperty("StartLatitude", sl);
        SetProperty("EndLatitude", el);
        SetProperty("Degrees", d);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SphereModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "SphereModel::ImportXlightsModel");
        return true;
    } else {
        DisplayError("Failure loading Sphere model file.");
        return false;
    }
}

void SphereModel::ExportAsCustomXModel3D() const
{

    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty()) return;

    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) {
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
        return;
    }

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

    int scaleFactor3D = 2;
    while (!Find3DCustomModelScale(scaleFactor3D, minx, miny, minz, w, h, d)) {
        ++scaleFactor3D;
        if (scaleFactor3D > 20) { // Using 2D technique from Scott
            scaleFactor3D = 2;
            break;
        }
    }

    std::vector<std::vector<std::vector<int>>> data;
    for (int l = 0; l < BufferWi * scaleFactor3D + 1; l++)
    {
        std::vector<std::vector<int>> layer;
        for (int r = BufferHt * scaleFactor3D + 1; r >= 0; r--)
        {
            std::vector<int> row;
            for (int c = 0; c < BufferWi * scaleFactor3D + 1; c++)
            {
                row.push_back(-1);
            }
            layer.push_back(row);
        }
        data.push_back(layer);
    }

    int i = 1;
    for (auto& n: Nodes)
    {
        int xx = (scaleFactor3D * (float)BufferWi) * (n->Coords[0].screenX - minx) / w;
        int yy = (scaleFactor3D * (float)BufferHt) - (scaleFactor3D * (float)BufferHt * (n->Coords[0].screenY - miny) / h);
        int zz = (scaleFactor3D * (float)BufferWi) * (n->Coords[0].screenZ - minz) / d;
        wxASSERT(xx >= 0 && xx < scaleFactor3D * BufferWi + 1);
        wxASSERT(yy >= 0 && yy < scaleFactor3D * BufferHt + 1);
        wxASSERT(zz >= 0 && zz < scaleFactor3D * BufferWi + 1);
        wxASSERT(data[zz][yy][xx] == -1);
        data[zz][yy][xx] = i++;
    }

    wxString p1 = wxString::Format("%i", (int)(scaleFactor3D * BufferWi + 1));
    wxString p2 = wxString::Format("%i", (int)(scaleFactor3D * BufferHt + 1));
    wxString dd = wxString::Format("%i", (int)(scaleFactor3D * BufferWi + 1));
    wxString p3 = wxString::Format("%i", parm3);
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency", "0");
    wxString mb = ModelXml->GetAttribute("ModelBrightness", "0");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString pc = ModelXml->GetAttribute("PixelCount");
    wxString pt = ModelXml->GetAttribute("PixelType");
    wxString psp = ModelXml->GetAttribute("PixelSpacing");
    wxString sl = ModelXml->GetAttribute("StartLatitude");
    wxString el = ModelXml->GetAttribute("EndLatitude");
    wxString dg = ModelXml->GetAttribute("Degrees");

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
    f.Write(wxString::Format("StartLatitude=\"%s\" ", sl));
    f.Write(wxString::Format("EndLatitude=\"%s\" ", el));
    f.Write(wxString::Format("Degrees=\"%s\" ", dg));
    if (pc != "")
        f.Write(wxString::Format("PixelCount=\"%s\" ", pc));
    if (pt != "")
        f.Write(wxString::Format("PixelType=\"%s\" ", pt));
    if (psp != "")
        f.Write(wxString::Format("PixelSpacing=\"%s\" ", psp));
    f.Write("CustomModel=\"");
    f.Write(CustomModel::ToCustomModel(data));
    f.Write("\" ");
    f.Write("CustomModelCompressed=\"");
    f.Write(CustomModel::ToCompressed(data));
    f.Write("\" ");
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
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
    ExportDimensions(f);
    f.Write("</custommodel>");
    f.Close();
}

bool SphereModel::Find3DCustomModelScale(int scale, float minx, float miny, float minz, float w, float h, float d) const
{
    size_t nodeCount = GetNodeCount();
    if (nodeCount <= 1) {
        return true;
    }
    for (int i = 0; i < nodeCount; ++i) {
        for (int j = i + 1; j < nodeCount; ++j) {
            int x1 = (scale * (float)BufferWi) * (Nodes[i]->Coords[0].screenX - minx) / w;
            int y1 = (scale * (float)BufferHt) - (scale * (float)BufferHt * (Nodes[i]->Coords[0].screenY - miny) / h);
            int z1 = (scale * (float)BufferWi) * (Nodes[i]->Coords[0].screenZ - minz) / d;

            int x2 = (scale * (float)BufferWi) * (Nodes[j]->Coords[0].screenX - minx) / w;
            int y2 = (scale * (float)BufferHt) - (scale * (float)BufferHt * (Nodes[j]->Coords[0].screenY - miny) / h);
            int z2 = (scale * (float)BufferWi) * (Nodes[j]->Coords[0].screenZ - minz) / d;

            if (x1 == x2 && y1 == y2 && z1 == z2) {
                return false;
            }
        }
    }
    return true;
}
