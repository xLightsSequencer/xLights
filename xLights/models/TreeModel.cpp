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

#include "TreeModel.h"
#include "ModelScreenLocation.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"
#include "../ModelPreview.h"
#include "CustomModel.h"

TreeModel::TreeModel(const ModelManager &manager) : MatrixModel(manager)
{
    screenLocation.SetSupportsZScaling(true);
    screenLocation.SetPreferredSelectionPlane(ModelScreenLocation::MSLPLANE::GROUND);
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
    if (_firstStrand < 0) {
        _firstStrand = 0;
    }
    if (_vMatrix) {
        InitVMatrix(_firstStrand);
    } else {
        InitHMatrix();
    }

    screenLocation.SetPerspective2D(_perspective);
    SetTreeCoord(_degrees);
    InitSingleChannelModel();
}

// initialize screen coordinates for tree
void TreeModel::SetTreeCoord(long _degrees)
{
    double bufferX, bufferY;
    if (BufferWi < 1)
        return;
    if (BufferHt < 1)
        return; // June 27,2013. added check to not divide by zero
    double RenderHt, RenderWi;
    if (_degrees > 0) {
        RenderHt = BufferHt * 3;
        RenderWi = ((double)RenderHt) / 1.8;

        double radians = toRadians(_degrees);
        double radius = RenderWi / 2.0;
        double topradius = radius;
        if (_botTopRatio != 0.0f) {
            topradius = radius / std::abs(_botTopRatio);
        }
        if (_botTopRatio < 0.0f) {
            std::swap(topradius, radius);
        }

        double StartAngle = -radians / 2.0;
        double AngleIncr = radians / double(BufferWi);
        if (_degrees < 350 && BufferWi > 1) {
            AngleIncr = radians / double(BufferWi - 1);
        }

        // shift a tiny bit to make the strands in back not line up exactly with the strands in front
        StartAngle += toRadians(_rotation);

        std::vector<float> yPos(BufferHt);
        std::vector<float> xInc(BufferHt);
        for (size_t x = 0; x < BufferHt; ++x) {
            yPos[x] = x;
            xInc[x] = 0;
        }
        if (_spiralRotations != 0.0f) {
            std::vector<float> lengths(10);
            float rgap = (radius - topradius) / 10.0;
            float total = 0;
            for (size_t x = 0; x < 10; ++x) {
                lengths[x] = 2.0 * M_PI * (radius - rgap * x) - rgap / 2.0;
                lengths[x] *= _spiralRotations / 10.0;
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
                float ang = _spiralRotations * 2.0 * M_PI / 10.0;
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
        double treeScale = _degrees == -1 ? 5.0 : 4.0;
        double botWid = BufferWi * treeScale;
        RenderHt = BufferHt * 2.0;
        RenderWi = (botWid + 2);

        double offset = 0.5;
        size_t NodeCount = GetNodeCount();
        for (size_t n = 0; n < NodeCount; n++) {
            size_t CoordCount = GetCoordCount(n);
            if (_degrees == -1) {
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
                    NodeBaseClass::CoordStruct cs = Nodes[n]->Coords[c];
                    cs.screenX = xb + (xt - xb) * posOnString;
                    cs.screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;
                    Nodes[n]->Coords.push_back(cs);

                    posOnString = 1;
                    if (BufferHt > 1) {
                        posOnString = ((bufferY + 0.33) / (double)(BufferHt - 1.0));
                    }
                    newh = RenderHt * posOnString;
                    cs = Nodes[n]->Coords[c];
                    cs.screenX = xb + (xt - xb) * posOnString;
                    cs.screenY = RenderHt * newh / h - ((double)RenderHt) / 2.0;
                    Nodes[n]->Coords.push_back(cs);
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
        _treeType = event.GetPropertyValue().GetLong();
        wxPGProperty *p = grid->GetPropertyByName("TreeDegrees");
        if (p != nullptr) {
            p->Enable(_treeType == 0);
        }
        p = grid->GetPropertyByName("TreeRotation");
        if (p != nullptr) {
            p->Enable(_treeType == 0);
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeStyle");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeStyle");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeStyle");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeStyle");
        return 0;
    } else if (event.GetPropertyName() == "TreeDegrees") {
        _degrees = (int)event.GetPropertyValue().GetLong();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeDegrees");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeDegrees");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeDegrees");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeDegrees");
        return 0;
    } else if (event.GetPropertyName() == "TreeRotation") {
        _rotation = (float)event.GetPropertyValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeRotation");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeRotation");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeRotation");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeRotation");
        return 0;
    } else if (event.GetPropertyName() == "TreeSpiralRotations") {
        _spiralRotations = (float)event.GetPropertyValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeSpiralRotations");
        return 0;
    } else if (event.GetPropertyName() == "TreeBottomTopRatio") {
        _botTopRatio = (float)event.GetPropertyValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreeBottomTopRatio");
        return 0;
    } else if (event.GetPropertyName() == "TreePerspective") {
        _perspective = (float)(event.GetPropertyValue().GetDouble()/10.0);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TreeModel::OnPropertyGridChange::TreePerspective");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TreeModel::OnPropertyGridChange::TreePerspective");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "TreeModel::OnPropertyGridChange::TreePerspective");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TreeModel::OnPropertyGridChange::TreePerspective");
        return 0;
    } else if ("StrandDir" == event.GetPropertyName()) {
        _vMatrix =  event.GetPropertyValue().GetBool();
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
    grid->Append(new wxEnumProperty("Type", "TreeStyle", TREE_STYLES, _treeType));

    wxPGProperty *p = grid->Append(new wxUIntProperty("Degrees", "TreeDegrees", _treeType == 0 ? _degrees : 180));
    p->SetAttribute("Min", "1");
    p->SetAttribute("Max", "360");
    p->SetEditor("SpinCtrl");
    p->Enable(_treeType == 0);

    p = grid->Append(new wxFloatProperty("Rotation", "TreeRotation", _treeType == 0 ? _rotation : 3));
    p->SetAttribute("Min", "-360");
    p->SetAttribute("Max", "360");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(_treeType == 0);

    p = grid->Append(new wxFloatProperty("Spiral Wraps", "TreeSpiralRotations", _treeType == 0 ? _spiralRotations : 0.0));
    p->SetAttribute("Min", "-200");
    p->SetAttribute("Max", "200");
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(_treeType == 0);

    p = grid->Append(new wxFloatProperty("Bottom/Top Ratio", "TreeBottomTopRatio", _treeType == 0 ? _botTopRatio : 6.0));
    p->SetAttribute("Min", "-50");
    p->SetAttribute("Max", "50");
    p->SetAttribute("Step", 0.5);
    p->SetAttribute("Precision", 2);
    p->SetEditor("SpinCtrl");
    p->Enable(_treeType == 0);

    p = grid->Append(new wxFloatProperty("Perspective", "TreePerspective", _treeType == 0 ? _perspective*10 : 2));
    p->SetAttribute("Min", "0");
    p->SetAttribute("Max", "10");
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
    p->Enable(_treeType == 0);

    p = grid->Append(new wxBoolProperty("Alternate Nodes", "AlternateNodes", _alternateNodes));
    p->SetEditor("CheckBox");
    p->Enable(_noZigZag == false);

    p = grid->Append(new wxBoolProperty("Don't Zig Zag", "NoZig", _noZigZag));
    p->SetEditor("CheckBox");
    p->Enable(_alternateNodes == false);

    grid->Append(new wxEnumProperty("Strand Direction", "StrandDir", TREE_DIRECTIONS, _vMatrix ? 1 : 0));
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

    int i = 1;
    for (auto& n : Nodes) {
        int xx = SCALE_FACTOR_3D * w * (n->Coords[0].screenX - minx) / w;
        int yy = (SCALE_FACTOR_3D * h) - (SCALE_FACTOR_3D * h * (n->Coords[0].screenY - miny) / h);
        int zz = SCALE_FACTOR_3D * d * (maxz - n->Coords[0].screenZ) / d;
        wxASSERT(xx >= 0 && xx < SCALE_FACTOR_3D * w + 1);
        wxASSERT(yy >= 0 && yy < SCALE_FACTOR_3D * h + 1);
        wxASSERT(zz >= 0 && zz < SCALE_FACTOR_3D * d + 1);
        wxASSERT(data[zz][yy][xx] == -1);
        data[zz][yy][xx] = i++;
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
    f.Write(CustomModel::ToCustomModel(data));
    f.Write("\" ");
    f.Write("CustomModelCompressed=\"");
    f.Write(CustomModel::ToCompressed(data));
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
