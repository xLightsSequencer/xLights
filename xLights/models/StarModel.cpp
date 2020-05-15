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
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/log.h>
#include <wx/msgdlg.h>

#include "StarModel.h"
#include "../xLightsVersion.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

std::vector<std::string> StarModel::STAR_BUFFER_STYLES;

StarModel::StarModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased) : ModelWithScreenLocation(manager), starRatio(2.618034f)
{
    SetFromXml(node, zeroBased);
}

StarModel::~StarModel()
{
}

const std::vector<std::string> &StarModel::GetBufferStyles() const {
    struct Initializer {
        Initializer() {
            STAR_BUFFER_STYLES = Model::DEFAULT_BUFFER_STYLES;
            STAR_BUFFER_STYLES.push_back("Layer Star");
        }
    };
    static Initializer ListInitializationGuard;
    return STAR_BUFFER_STYLES;
}

void StarModel::GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform, int &BufferWi, int &BufferHi) const {
    if (type == "Layer Star") {
        BufferHi = GetNumStrands();
        BufferWi = 0;
        for (int x = 0; x < BufferHi; x++) {
            int w = GetStarSize(x);
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        AdjustForTransform(transform, BufferWi, BufferHi);
    }
    else {
        Model::GetBufferSize(type, camera, transform, BufferWi, BufferHi);
    }
}

void StarModel::InitRenderBufferNodes(const std::string &type,
                                      const std::string &camera,
                                      const std::string &transform,
                                      std::vector<NodeBaseClassPtr> &newNodes, int &BufferWi, int &BufferHi) const {
    if (type == "Layer Star") {
        BufferHi = GetNumStrands();
        BufferWi = 0;
        for (int x = 0; x < BufferHi; x++) {
            int w = GetStarSize(x);
            if (w > BufferWi) {
                BufferWi = w;
            }
        }
        for (int x = 0; x < BufferHi; x++) {
            int w = GetStarSize(x);
            for (int z = 0; z < w; z++) {

            }
        }
        for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
            newNodes.push_back(NodeBaseClassPtr(it->get()->clone()));
        }

        int start = 0;
        for (int cur = 0; cur < starSizes.size(); cur++) {
            int numlights = starSizes[cur];
            if (numlights == 0) {
                continue;
            }

            for(size_t cnt=0; cnt<numlights; cnt++) {
                int n;
                if (!SingleNode) {
                    n = cnt + start;
                } else {
                    n = cur;
                    if (n >= Nodes.size()) {
                        n = Nodes.size() - 1;
                    }
                }
                for (auto it = newNodes[n]->Coords.begin(); it != newNodes[n]->Coords.end(); ++it) {
                    it->bufY = cur;
                    it->bufX = cnt * BufferWi / numlights;
                }
            }
            start += numlights;
        }
        ApplyTransform(transform, newNodes, BufferWi, BufferHi);
    } else {
        Model::InitRenderBufferNodes(type, camera, transform, newNodes, BufferWi, BufferHi);
    }
}

int StarModel::GetStrandLength(int strand) const {
    return SingleNode ? 1 : GetStarSize(strand);
}

int StarModel::MapToNodeIndex(int strand, int node) const {
    int idx = 0;
    for (int x = 0; x < strand; x++) {
        idx += GetStrandLength(x);
    }
    idx += node;
    return idx;
}

int StarModel::GetNumStrands() const {
    return starSizes.size();
}

bool StarModel::AllNodesAllocated() const 
{
    int allocated = 0;
    for (const auto& it : starSizes)
    {
        allocated += it;
    }

    return (allocated == GetNodeCount());
}

// parm3 is number of points
// top left=top ccw, top right=top cw, bottom left=bottom cw, bottom right=bottom ccw

void StarModel::InitModel()
{
    starRatio = wxAtof(ModelXml->GetAttribute("starRatio", "2.618034"));

    wxString tempstr = ModelXml->GetAttribute("starSizes");
    starSizes.resize(0);
    while (tempstr.size() > 0) {
        wxString t2 = tempstr;
        if (tempstr.Contains(",")) {
            t2 = tempstr.SubString(0, tempstr.Find(","));
            tempstr = tempstr.SubString(tempstr.Find(",") + 1, tempstr.length());
        }
        else {
            tempstr = "";
        }
        long i2 = 0;
        t2.ToLong(&i2);
        if (i2 > 0) {
            starSizes.resize(starSizes.size() + 1);
            starSizes[starSizes.size() - 1] = i2;
        }
    }

    if (parm3 < 2) parm3 = 2; // need at least 2 arms
    SetNodeCount(parm1, parm2, rgbOrder);

    // Found a problem where a user had multiple layer sizes but just 1 string and set to RGB dumb string type.
    // I think the commented out code would fix this but I am not sure it would work in all situations.
    // It needs more testing and late november is not a good time to be doing it. So throwing an assertion in
    // If this fires for us a lot when there is nothing wrong with our models then we will know the code is bad and we wont implement it.
    // Maybe you can help fix the condition at that time ... rather than just commenting out the assert.
    // wxASSERT(starSizes.size() <= Nodes.size());
    //if (starSizes.size() > Nodes.size())
    //{
    //    starSizes.resize(Nodes.size());
    //}

    int maxLights = 0;
    int numlights = parm1 * parm2;
    int cnt = 0;
    if (starSizes.size() == 0) {
        starSizes.resize(1);
        starSizes[0] = numlights;
    }

    bool duplicateSized = false;
    std::list<int> duplicateSizedLayers;
    for (int x = 0; x < starSizes.size(); x++) {

        if (std::find(begin(duplicateSizedLayers), end(duplicateSizedLayers), starSizes[x]) == duplicateSizedLayers.end()) {
            duplicateSizedLayers.push_back(starSizes[x]);
        }
        else {
            duplicateSized = true;
        }

        if ((cnt + starSizes[x]) > numlights) {
            starSizes[x] = numlights - cnt;
        }
        cnt += starSizes[x];
        if (starSizes[x] > maxLights) {
            maxLights = starSizes[x];
        }
    }

    // This is used to separate layers with equal numbers of nodes
    const int SEPERATION_FACTOR = 10;
    if (duplicateSized) {
        int size = maxLights + 1 + SEPERATION_FACTOR * starSizes.size();
        SetBufferSize(size, size);
    }
    else {
        SetBufferSize(maxLights + 1, maxLights + 1);
    }
    int LastStringNum = -1;
    int chan = 0;
    int start = 0;
    double scale = (double)(maxLights + 1) / (double)(maxLights + 1 + SEPERATION_FACTOR);

    for (int cur = 0; cur < starSizes.size(); cur++) {
        numlights = starSizes[cur];
        if (numlights == 0) {
            continue;
        }

        double offset = 0.0;
        double coffset = 0.0;

        if (duplicateSized) {
            // we do funky things if there are duplicate layers with the same number of pixels as the orignal code 
            // forced the pixels into order largest to smallest ... at least on the display
            // I have kept the old code for stars without duplicate sized layers to minimise impacts on users.
            double sep = (double)(SEPERATION_FACTOR * (starSizes.size() - cur));
            offset = ((double)(numlights) / 2.0 + sep) * scale;
            coffset = ((double)(maxLights - numlights) / 2.0 - sep) * scale;
        }
        else {
            offset = (double)numlights / 2.0;
            coffset = (double)(maxLights - numlights) / 2.0;
        }

        int numsegments = parm3 * 2;
        double dpct = 1.0 / (double)numsegments;
        double OuterRadius = offset;
        double InnerRadius = OuterRadius / starRatio; // divide by ratio (default is golden ratio squared)
        double pct = isBotToTop ? 0.5 : 0.0;          // % of circle, 0=top
        double pctIncr = 1.0 / (double)numlights;     // this is cw
        if (IsLtoR != isBotToTop) pctIncr *= -1.0;    // adjust to ccw
        int ChanIncr = GetNodeChannelCount(StringType);
        for (size_t cnt2 = 0; cnt2 < numlights; cnt2++) {
            int n;
            if (!SingleNode) {
                n = start + cnt2;
            }
            else {
                n = cur;
                if (n >= Nodes.size()) {
                    n = Nodes.size() - 1;
                }
            }
            if (Nodes[n]->StringNum != LastStringNum) {
                LastStringNum = Nodes[n]->StringNum;
                chan = stringStartChan[LastStringNum];
            }
            Nodes[n]->ActChan = chan;
            if (!SingleNode) {
                chan += ChanIncr;
            }
            size_t CoordCount = GetCoordCount(n);
            int lastx = 0;
            int lasty = 0;
            if (duplicateSized) {
                lastx = offset;
                lasty = offset;
            }
            for (size_t c = 0; c < CoordCount; c++) {
                if (c >= numlights) {
                    Nodes[n]->Coords[c].bufX = lastx;
                    Nodes[n]->Coords[c].bufY = lasty;
                }
                else {
                    int cursegment = (int)((double)numsegments * pct) % numsegments;
                    int nextsegment = (cursegment + 1) % numsegments;
                    double segstart_pct = (double)cursegment / numsegments;
                    double segend_pct = (double)nextsegment / numsegments;
                    double dseg = pct - segstart_pct;
                    double segpct = dseg / dpct;
                    double r = cursegment % 2 == 0 ? OuterRadius : InnerRadius;
                    double segstart_x = r * sin(segstart_pct * 2.0 * M_PI);
                    double segstart_y = r * cos(segstart_pct * 2.0 * M_PI);
                    r = nextsegment % 2 == 0 ? OuterRadius : InnerRadius;
                    double segend_x = r * sin(segend_pct * 2.0 * M_PI);
                    double segend_y = r * cos(segend_pct * 2.0 * M_PI);
                    // now interpolate between segstart and segend
                    int x = (segend_x - segstart_x) * segpct + segstart_x + offset + 0.5 + coffset; 
                    int y = (segend_y - segstart_y) * segpct + segstart_y + offset + 0.5 + coffset;
                    if (duplicateSized)                         {
                        x += (SEPERATION_FACTOR * starSizes.size()) / 2;
                        y += (SEPERATION_FACTOR * starSizes.size()) / 2;
                    }
                    Nodes[n]->Coords[c].bufX = x;
                    Nodes[n]->Coords[c].bufY = y;
                    lastx = x;
                    lasty = y;
                    pct += pctIncr;
                    if (pct >= 1.0) pct -= 1.0;
                    if (pct < 0.0) pct += 1.0;
                }
            }
        }
        start += numlights;
    }

    CopyBufCoord2ScreenCoord();
    screenLocation.RenderDp = 10.0f;  // give the bounding box a little depth
}

static const char* TOP_BOT_LEFT_RIGHT_VALUES[] = { 
        "Top Ctr-CCW",
        "Top Ctr-CW",
        "Bottom Ctr-CW",
        "Bottom Ctr-CCW"
};
static wxPGChoices TOP_BOT_LEFT_RIGHT(wxArrayString(4, TOP_BOT_LEFT_RIGHT_VALUES));

void StarModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    wxPGProperty *p = grid->Append(new wxUIntProperty("# Strings", "StarStringCount", parm1));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 640);
    p->SetEditor("SpinCtrl");

    if (SingleNode) {
        p = grid->Append(new wxUIntProperty("Lights/String", "StarLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    } else {
        p = grid->Append(new wxUIntProperty("Nodes/String", "StarLightCount", parm2));
        p->SetAttribute("Min", 1);
        p->SetAttribute("Max", 10000);
        p->SetEditor("SpinCtrl");
    }

    p = grid->Append(new wxUIntProperty("# Points", "StarStrandCount", parm3));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 250);
    p->SetEditor("SpinCtrl");

    grid->Append(new wxEnumProperty("Starting Location", "StarStart", TOP_BOT_LEFT_RIGHT, IsLtoR ? (isBotToTop ? 2 : 0) : (isBotToTop ? 3 : 1)));
    grid->Append(new wxStringProperty("Layer Sizes", "StarLayerSizes", ModelXml->GetAttribute("starSizes")));

    p = grid->Append(new wxFloatProperty("Outer to Inner Ratio", "StarRatio",  starRatio));
    p->SetAttribute("Precision", 2);
    p->SetAttribute("Step", 0.1);
    p->SetEditor("SpinCtrl");
}

int StarModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("StarStringCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm1");
        ModelXml->AddAttribute("parm1", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        //AdjustStringProperties(grid, parm1);
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnPropertyGridChange::StarStringCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnPropertyGridChange::StarStringCount");
        return 0;
    } else if ("StarLightCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm2");
        ModelXml->AddAttribute("parm2", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnPropertyGridChange::StarLightCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnPropertyGridChange::StarLightCount");
        return 0;
    } else if ("StarStrandCount" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("parm3");
        ModelXml->AddAttribute("parm3", wxString::Format("%d", (int)event.GetPropertyValue().GetLong()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODELLIST, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "StarModel::OnPropertyGridChange::StarStrandCount");
        AddASAPWork(OutputModelManager::WORK_MODELS_REWORK_STARTCHANNELS, "StarModel::OnPropertyGridChange::StarStrandCount");
        return 0;
    } else if ("StarStart" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Dir");
        ModelXml->AddAttribute("Dir", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 2 ? "L" : "R");
        ModelXml->DeleteAttribute("StartSide");
        ModelXml->AddAttribute("StartSide", event.GetValue().GetLong() == 0 || event.GetValue().GetLong() == 1 ? "T" : "B");
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarStart");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarStart");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarStart");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarStart");
        return 0;
    } else if ("StarLayerSizes" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("starSizes");
        ModelXml->AddAttribute("starSizes", event.GetValue().GetString());
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarLayerSizes");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarLayerSizes");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarLayerSizes");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarLayerSizes");
        return 0;
    } else if ("StarRatio" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("starRatio");
        ModelXml->AddAttribute("starRatio", wxString::Format("%lf", event.GetValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::OnPropertyGridChange::StarRatio");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::OnPropertyGridChange::StarRatio");
        AddASAPWork(OutputModelManager::WORK_RELOAD_MODEL_FROM_XML, "StarModel::OnPropertyGridChange::StarRatio");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "StarModel::OnPropertyGridChange::StarRatio");
        return 0;
    }

    return Model::OnPropertyGridChange(grid, event);
}

void StarModel::ExportXlightsModel()
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
    wxString ss = ModelXml->GetAttribute("starSizes");
    wxString sr = ModelXml->GetAttribute("starRatio","2.618034");
    wxString sts = ModelXml->GetAttribute("StartSide");
    wxString dir = ModelXml->GetAttribute("Dir");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString da = ModelXml->GetAttribute("DisplayAs");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<starmodel \n");
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
    f.Write(wxString::Format("StartSide=\"%s\" ", sts));
    f.Write(wxString::Format("starSizes=\"%s\" ", ss));
    f.Write(wxString::Format("starRatio=\"%s\" ", sr));
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
    f.Write("</starmodel>");
    f.Close();
}

void StarModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "starmodel")
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
            wxString sts = root->GetAttribute("StartSide");
            wxString ss = root->GetAttribute("starSizes");
            wxString sr = root->GetAttribute("starRatio");
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
			if (sr.IsEmpty())
				sr = "2.618034";

            SetProperty("parm1", p1);
            SetProperty("parm2", p2);
            SetProperty("parm3", p3);
            SetProperty("StringType", st);
            SetProperty("PixelSize", ps);
            SetProperty("Transparency", t);
            SetProperty("ModelBrightness", mb);
            SetProperty("Antialias", a);
            SetProperty("StartSide", sts);
            SetProperty("starSizes", ss);
            SetProperty("starRatio", sr);
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

            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "StarModel::ImportXlightsModel");
            xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "StarModel::ImportXlightsModel");
        }
        else
        {
            DisplayError("Failure loading Star model file.");
        }
    }
    else
    {
        DisplayError("Failure loading Star model file.");
    }
}
