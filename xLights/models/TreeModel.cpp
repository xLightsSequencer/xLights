/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/tokenzr.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/log.h>
#include <wx/filedlg.h>

#include "TreeModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"

TreeModel::TreeModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : MatrixModel(manager)
{
    treeType = 0;
    degrees = 360;
    screenLocation.SetSupportsZScaling(true);
    screenLocation.SetPreferredSelectionPlane(ModelScreenLocation::MSLPLANE::GROUND);
    SetFromXml(node, zeroBased);
}

TreeModel::~TreeModel()
{
}

static const char* TREE_DIRECTION_VALUES[] = {
    "Horizontal",
    "Vertical"
};
static wxPGChoices TREE_DIRECTIONS(wxArrayString(2, TREE_DIRECTION_VALUES));

void TreeModel::InitModel() {
    _alternateNodes = (ModelXml->GetAttribute("AlternateNodes", "false") == "true");
    _noZig = (ModelXml->GetAttribute("NoZig", "false") == "true");
    bool isHMatrix = (ModelXml->GetAttribute("StrandDir", TREE_DIRECTION_VALUES[1]) == TREE_DIRECTION_VALUES[0]);
    wxStringTokenizer tkz(DisplayAs, " ");
    wxString token = tkz.GetNextToken();

    int firstStrand = 0;
    if (zeroBased && ModelXml->GetAttribute("exportFirstStrand") != "") {
        firstStrand = wxAtoi(ModelXml->GetAttribute("exportFirstStrand")) - 1;
    }
    if (firstStrand < 0) {
        firstStrand = 0;
    }
    if (isHMatrix) {
        InitHMatrix();
    } else {
        InitVMatrix(firstStrand);
    }
    token = tkz.GetNextToken();
    token.ToLong(&degrees);
    treeType = 0;
    if (token == "Flat") {
        treeType = 1;
        degrees = 0;
    } else if (token == "Ribbon") {
        treeType = 2;
        degrees = -1;
    }
    rotation = wxAtof(ModelXml->GetAttribute("TreeRotation", "3"));
    spiralRotations = wxAtof(ModelXml->GetAttribute("TreeSpiralRotations", "0.0"));
    botTopRatio = wxAtof(ModelXml->GetAttribute("TreeBottomTopRatio", "6.0"));
    perspective =  wxAtof(ModelXml->GetAttribute("TreePerspective", "0.2"));
    screenLocation.SetPerspective2D(perspective);
    SetTreeCoord(degrees);
    InitSingleChannelModel();
    DisplayAs = "Tree";
}

// initialize screen coordinates for tree
void TreeModel::SetTreeCoord(long degrees)
{
    double bufferX, bufferY;
    if (BufferWi < 1)
        return;
    if (BufferHt < 1)
        return; // June 27,2013. added check to not divide by zero
    double RenderHt, RenderWi;
    if (degrees > 0) {
        RenderHt = BufferHt * 3;
        RenderWi = ((double)RenderHt) / 1.8;

        double radians = toRadians(degrees);
        double radius = RenderWi / 2.0;
        double topradius = radius;
        if (botTopRatio != 0.0f) {
            topradius = radius / std::abs(botTopRatio);
        }
        if (botTopRatio < 0.0f) {
            std::swap(topradius, radius);
        }

        double StartAngle = -radians / 2.0;
        double AngleIncr = radians / double(BufferWi);
        if (degrees < 350 && BufferWi > 1) {
            AngleIncr = radians / double(BufferWi - 1);
        }

        // shift a tiny bit to make the strands in back not line up exactly with the strands in front
        StartAngle += toRadians(rotation);

        std::vector<float> yPos(BufferHt);
        std::vector<float> xInc(BufferHt);
        for (size_t x = 0; x < BufferHt; ++x) {
            yPos[x] = x;
            xInc[x] = 0;
        }
        if (spiralRotations != 0.0f) {
            std::vector<float> lengths(10);
            float rgap = (radius - topradius) / 10.0;
            float total = 0;
            for (size_t x = 0; x < 10; ++x) {
                lengths[x] = 2.0 * M_PI * (radius - rgap * x) - rgap / 2.0;
                lengths[x] *= spiralRotations / 10.0;
                lengths[x] = sqrt(lengths[x] * lengths[x] + (float)BufferHt / 10.0 * (float)BufferHt / 10.0);
                total += lengths[x];
            }
            int lights = 0;
            for (int x = 0; x < 10; x++) {
                lengths[x] /= total;
                lights += (int)std::round(lengths[x] * BufferHt);
            }
            int curSeg = 0;
            float lightsInSeg = std::round(lengths[0] * BufferHt);
            int curLightInSeg = 0;
            for (int x = 1; x < BufferHt; x++) {
                if (curLightInSeg >= lightsInSeg) {
                    curSeg++;
                    curLightInSeg = 0;
                    if (curSeg == 9) {
                        lightsInSeg = BufferHt - x;
                    } else {
                        lightsInSeg = std::round(lengths[curSeg] * BufferHt);
                    }
                }
                float ang = spiralRotations * 2.0 * M_PI / 10.0;
                ang /= (float)lightsInSeg;
                yPos[x] = yPos[x - 1] + (BufferHt / 10.0 / lightsInSeg);
                xInc[x] = xInc[x - 1] + ang;
                curLightInSeg++;
            }
        }

        double topYoffset = 0.0; // std::abs(perspective * topradius * cos(M_PI));
        double ytop = RenderHt - topYoffset;
        double ybot = 0.0; // std::abs(perspective * radius * cos(M_PI));

        size_t NodeCount = GetNodeCount();
        for (size_t n = 0; n < NodeCount; n++) {
            size_t CoordCount = GetCoordCount(n);
            for (size_t c = 0; c < CoordCount; c++) {
                bufferX = Nodes[n]->Coords[c].bufX;
                bufferY = Nodes[n]->Coords[c].bufY;
                double angle = StartAngle + double(bufferX) * AngleIncr + xInc[bufferY];
                double xb = radius * sin(angle);
                double xt = topradius * sin(angle);
                double zb = radius * cos(angle);
                double zt = topradius * cos(angle);
                double yb = ybot;
                double yt = ytop;
                // double yb = ybot - perspective * radius * cos(angle);
                // double yt = ytop - perspective * topradius * cos(angle);
                double posOnString = 0.5;
                if (BufferHt > 1) {
                    posOnString = yPos[bufferY] / (double)(BufferHt - 1.0);
                }

                Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                Nodes[n]->Coords[c].screenY = yb + (yt - yb) * posOnString - ((double)RenderHt) / 2.0;
                Nodes[n]->Coords[c].screenZ = zb + (zt - zb) * posOnString;
            }
        }
    } else {
        double treeScale = degrees == -1 ? 5.0 : 4.0;
        double botWid = BufferWi * treeScale;
        RenderHt = BufferHt * 2.0;
        RenderWi = (botWid + 2);

        double offset = 0.5;
        size_t NodeCount = GetNodeCount();
        for (size_t n = 0; n < NodeCount; n++) {
            size_t CoordCount = GetCoordCount(n);
            if (degrees == -1) {
                for (size_t c = 0; c < CoordCount; c++) {
                    bufferX = Nodes[n]->Coords[c].bufX;
                    bufferY = Nodes[n]->Coords[c].bufY;

                    double xt = (bufferX + offset - BufferWi / 2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi / 2.0) * treeScale;
                    double h = std::sqrt(RenderHt * RenderHt + (xt - xb) * (xt - xb));

                    double posOnString = 0.5;
                    if (BufferHt > 1) {
                        posOnString = (bufferY / (double)(BufferHt - 1.0));
                    }

                    double newh = RenderHt * posOnString;
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;

                    posOnString = 0;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY - 0.33) / (double)(BufferHt - 1.0));
                    }

                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;

                    posOnString = 1;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY + 0.33) / (double)(BufferHt - 1.0));
                    }
                    newh = RenderHt * posOnString;
                    Nodes[n]->Coords.push_back(Nodes[n]->Coords[c]);
                    Nodes[n]->Coords.back().screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords.back().screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;
                }

            } else {
                for (size_t c = 0; c < CoordCount; c++) {
                    bufferX = Nodes[n]->Coords[c].bufX;
                    bufferY = Nodes[n]->Coords[c].bufY;

                    double xt = (bufferX + offset - BufferWi / 2.0) * 0.9;
                    double xb = (bufferX + offset - BufferWi / 2.0) * treeScale;
                    double posOnString = 0.5;
                    if (BufferHt > 1) {
                        posOnString = (bufferY / (double)(BufferHt - 1.0));
                    }
                    Nodes[n]->Coords[c].screenX = xb + (xt - xb) * posOnString;
                    Nodes[n]->Coords[c].screenY = RenderHt * posOnString - ((double)RenderHt) / 2.0;
                }
            }
        }
    }
    screenLocation.SetRenderSize(RenderWi, RenderHt, RenderWi);
}

int TreeModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if (event.GetPropertyName() == "TreeStyle") {
        ModelXml->DeleteAttribute("DisplayAs");
        wxPGProperty *p = grid->GetPropertyByName("TreeDegrees");
        if (p != nullptr) {
            degrees = p->GetValue().GetLong();
        }
        switch (event.GetPropertyValue().GetLong()) {
            case 0:
                ModelXml->AddAttribute("DisplayAs", wxString::Format("Tree %d", degrees > 1 ? (int)degrees : 180));
                treeType = 0;
                break;
            case 1:
                ModelXml->AddAttribute("DisplayAs", "Tree Flat");
                treeType = 1;
                break;
            case 2:
                ModelXml->AddAttribute("DisplayAs", "Tree Ribbon");
                treeType = 2;
                break;
            default:
                wxASSERT(false);
                break;
        }
        if (p != nullptr) {
            p->Enable(treeType == 0);
        }
        p = grid->GetPropertyByName("TreeRotation");
        if (p != nullptr) {
            p->Enable(treeType == 0);
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeStyle");
        return 0;
    } else if (event.GetPropertyName() == "TreeDegrees") {
        ModelXml->DeleteAttribute("DisplayAs");
        ModelXml->AddAttribute("DisplayAs", wxString::Format("Tree %d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeDegrees");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeDegrees");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeDegrees");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeDegrees");
        return 0;
    } else if (event.GetPropertyName() == "TreeRotation") {
        ModelXml->DeleteAttribute("TreeRotation");
        ModelXml->AddAttribute("TreeRotation", wxString::Format("%f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeRotation");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeRotation");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeRotation");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeRotation");
        return 0;
    } else if (event.GetPropertyName() == "TreeSpiralRotations") {
        ModelXml->DeleteAttribute("TreeSpiralRotations");
        ModelXml->AddAttribute("TreeSpiralRotations", wxString::Format("%f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        return 0;
    } else if (event.GetPropertyName() == "TreeBottomTopRatio") {
        ModelXml->DeleteAttribute("TreeBottomTopRatio");
        ModelXml->AddAttribute("TreeBottomTopRatio", wxString::Format("%f", (float)event.GetPropertyValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        return 0;
    } else if (event.GetPropertyName() == "TreePerspective") {
        ModelXml->DeleteAttribute("TreePerspective");
        ModelXml->AddAttribute("TreePerspective", wxString::Format("%f", (float)(event.GetPropertyValue().GetDouble()/10.0)));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreePerspective");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreePerspective");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreePerspective");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreePerspective");
        return 0;
    } else if ("StrandDir" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("StrandDir");
        ModelXml->AddAttribute("StrandDir", event.GetPropertyValue().GetLong() ? TREE_DIRECTION_VALUES[1] : TREE_DIRECTION_VALUES[0]);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::StrandDir");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::StrandDir");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::StrandDir");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::StrandDir");
        return 0;
    }
    return MatrixModel::OnPropertyGridChange(grid, event);
}

static const char* TREE_STYLES_VALUES[] = { 
        "Round",
        "Flat",
        "Ribbon"
};
static wxPGChoices TREE_STYLES(wxArrayString(3, TREE_STYLES_VALUES));

void TreeModel::AddStyleProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxEnumProperty("Type", "TreeStyle", TREE_STYLES, treeType));

    wxPGProperty *p = grid->Append(new wxUIntProperty("Degrees", "TreeDegrees", treeType == 0 ? degrees : 180));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "360");
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);

    p = grid->Append(new wxFloatProperty("Rotation", "TreeRotation", treeType == 0 ? rotation : 3));
    p->SetAttribute("Min", "-360");
    p->SetAttribute("Max", "360");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);

    p = grid->Append(new wxFloatProperty("Spiral Wraps", "TreeSpiralRotations", treeType == 0 ? spiralRotations : 0.0));
    p->SetAttribute("Min", "-200");
    p->SetAttribute("Max", "200");
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);

    p = grid->Append(new wxFloatProperty("Bottom/Top Ratio", "TreeBottomTopRatio", treeType == 0 ? botTopRatio : 6.0));
    p->SetAttribute("Min", "-50");
    p->SetAttribute("Max", "50");
    p->SetAttribute("Step", 0.5);
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);

    p = grid->Append(new wxFloatProperty("Perspective", "TreePerspective", treeType == 0 ? perspective*10 : 2));
    p->SetAttribute("Min", "0");
    p->SetAttribute("Max", "10");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(treeType == 0);

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");
    p->Enable(_noZig == false);

    p = grid->Append(new wxBoolProperty("Don't Zig Zag", "NoZig", _noZig));
    p->SetEditor("CheckBox");
    p->Enable(_alternateNodes == false);

    grid->Append(new wxEnumProperty("Strand Direction", "StrandDir", TREE_DIRECTIONS, vMatrix ? 1 : 0));
}

void TreeModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());
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
    wxString tbtr = ModelXml->GetAttribute("TreeBottomTopRatio", "6.0");
    wxString tp = ModelXml->GetAttribute("TreePerspective", "0.2");
    wxString tr = ModelXml->GetAttribute("TreeRotation", "3");
    wxString tsr = ModelXml->GetAttribute("TreeSpiralRotations", "0.0");
    wxString an = ModelXml->GetAttribute("AlternateNodes", "false");
    wxString nz = ModelXml->GetAttribute("NoZig", "false");
    wxString sdr = ModelXml->GetAttribute("StrandDir", TREE_DIRECTION_VALUES[1]);
    wxString v = xlights_version_string;

    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<treemodel \n");
    f.Write(wxString::Format("AlternateNodes=\"%s\" ", an));
    f.Write(wxString::Format("NoZig=\"%s\" ", nz));
    f.Write(wxString::Format("StrandDir=\"%s\" ", sdr));
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
    f.Write(wxString::Format("TreeRotation=\"%s\" ", tr));
    f.Write(wxString::Format("TreeBottomTopRatio=\"%s\" ", tbtr));
    f.Write(wxString::Format("TreePerspective=\"%s\" ", tp));
    f.Write(wxString::Format("TreeSpiralRotations=\"%s\" ", tsr));
    f.Write(wxString::Format("SourceVersion=\"%s\" ", v));
    f.Write(ExportSuperStringColors());
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
    wxString groups = SerialiseGroups();
    if (groups != "") {
        f.Write(groups);
    }
    ExportDimensions(f);
    f.Write("</treemodel>");
    f.Close();
}

void TreeModel::ImportXlightsModel(wxXmlNode* root, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (root->GetName() == "treemodel") {
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
        wxString da = root->GetAttribute("DisplayAs");
        wxString tbtr = root->GetAttribute("TreeBottomTopRatio");
        wxString tp = root->GetAttribute("TreePerspective");
        wxString tr = root->GetAttribute("TreeRotation");
        wxString tsr = root->GetAttribute("TreeSpiralRotations");
        wxString pc = root->GetAttribute("PixelCount");
        wxString pt = root->GetAttribute("PixelType");
        wxString psp = root->GetAttribute("PixelSpacing");
        wxString an = root->GetAttribute("AlternateNodes");
        wxString nz = root->GetAttribute("NoZig");
        wxString sdr = root->GetAttribute("StrandDir");

        // Add any model version conversion logic here
        // Source version will be the program version that created the custom model
        if (sdr == "")
        {
            sdr = TREE_DIRECTION_VALUES[1];
        }

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
        SetProperty("TreeBottomTopRatio", tbtr);
        SetProperty("TreePerspective", tp);
        SetProperty("TreeRotation", tr);
        SetProperty("TreeSpiralRotations", tsr);
        SetProperty("PixelCount", pc);
        SetProperty("PixelType", pt);
        SetProperty("PixelSpacing", psp);
        SetProperty("AlternateNodes", an);
        SetProperty("NoZig", nz);
        SetProperty("StrandDir", sdr);

        wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
        GetModelScreenLocation().Write(ModelXml);
        SetProperty("name", newname, true);

        ImportSuperStringColours(root);
        ImportModelChildren(root, xlights, newname, min_x, max_x, min_y, max_y);

        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::ImportXlightsModel");
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::ImportXlightsModel");
    } else {
        DisplayError("Failure loading Tree model file.");
    }
}

#define SCALE_FACTOR_3D (1.1)
void TreeModel::ExportAsCustomXModel3D() const
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; // kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filename.IsEmpty())
        return;

    wxFile f(filename);
    //    bool isnew = !FileExists(filename);
    if (!f.Create(filename, true) || !f.IsOpened())
        DisplayError(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()).ToStdString());

    float minx = 99999;
    float miny = 99999;
    float minz = 99999;
    float maxx = -99999;
    float maxy = -99999;
    float maxz = -99999;

    for (auto& n : Nodes) {
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
    for (int l = 0; l < SCALE_FACTOR_3D * d + 1; l++) {
        std::vector<std::vector<int>> layer;
        for (int r = SCALE_FACTOR_3D * h + 1; r >= 0; r--) {
            std::vector<int> row;
            for (int c = 0; c < SCALE_FACTOR_3D * w + 1; c++) {
                row.push_back(-1);
            }
            layer.push_back(row);
        }
        data.push_back(layer);
    }

    uint32_t i = 0;
    for (auto& n : Nodes) {
        int xx = SCALE_FACTOR_3D * w * (n->Coords[0].screenX - minx) / w;
        int yy = (SCALE_FACTOR_3D * h) - (SCALE_FACTOR_3D * h * (n->Coords[0].screenY - miny) / h);
        int zz = SCALE_FACTOR_3D * d * (n->Coords[0].screenZ - minz) / d;
        wxASSERT(xx >= 0 && xx < SCALE_FACTOR_3D * w + 1);
        wxASSERT(yy >= 0 && yy < SCALE_FACTOR_3D * h + 1);
        wxASSERT(zz >= 0 && zz < SCALE_FACTOR_3D * d + 1);
        wxASSERT(data[zz][yy][xx] == -1);
        data[zz][yy][xx] = i++;
    }

    wxString cm = "";
    for (auto l : data) {
        if (cm != "")
            cm += "|";
        wxString ll = "";

        for (auto r : l) {
            if (ll != "")
                ll += ";";
            wxString rr = "";

            bool first = true;
            for (auto c : r) {
                if (first) {
                    first = false;
                } else {
                    rr += ",";
                }

                if (c != -1) {
                    rr += wxString::Format("%d ", c);
                }
            }
            ll += rr;
        }
        cm += ll;
    }

    wxString p1 = wxString::Format("%i", (int)(SCALE_FACTOR_3D * w + 1));
    wxString p2 = wxString::Format("%i", (int)(SCALE_FACTOR_3D * h + 1));
    wxString dd = wxString::Format("%i", (int)(SCALE_FACTOR_3D * d + 1));
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
    f.Write(ExportSuperStringColors());
    f.Write(" >\n");
    wxString face = SerialiseFace();
    if (face != "") {
        f.Write(face);
    }
    wxString state = SerialiseState();
    if (state != "") {
        f.Write(state);
    }
    wxString submodel = SerialiseSubmodel();
    if (submodel != "") {
        f.Write(submodel);
    }
    f.Write("</custommodel>");
    f.Close();
}
