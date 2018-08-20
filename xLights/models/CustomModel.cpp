#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "CustomModel.h"
#include "../CustomModelDialog.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"
#include "outputs/Output.h"
#include "UtilFunctions.h"
#include "outputs/OutputManager.h"

#include <log4cpp/Category.hh>

#define retmsg(msg)  \
{ \
wxMessageBox(msg, _("Export Error")); \
return; \
}

CustomModel::CustomModel(wxXmlNode *node, const ModelManager &manager,  bool zeroBased) : ModelWithScreenLocation(manager)
{
    SetFromXml(node, zeroBased);
}

CustomModel::~CustomModel()
{
}

static const std::string CLICK_TO_EDIT("--Click To Edit--");
class CustomModelDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    CustomModelDialogAdapter(CustomModel *model)
    : wxPGEditorDialogAdapter(), m_model(model) {
    }
    virtual bool DoShowDialog(wxPropertyGrid* propGrid,
                              wxPGProperty* WXUNUSED(property) ) override {
        CustomModelDialog dlg(propGrid);
        dlg.Setup(m_model);
        if (dlg.ShowModal() == wxID_OK) {
            dlg.Save(m_model);
            wxVariant v(CLICK_TO_EDIT);
            SetValue(v);
            return true;
        }
        return false;
    }
protected:
    CustomModel *m_model;
};


class CustomModelProperty : public wxStringProperty
{
public:
    CustomModelProperty(CustomModel *m,
                         const wxString& label,
                         const wxString& name,
                         const wxString& value)
    : wxStringProperty(label, name, value), m_model(m) {
    }
    // Set editor to have button
    virtual const wxPGEditor* DoGetEditorClass() const override {
        return wxPGEditor_TextCtrlAndButton;
    }
    // Set what happens on button click
    virtual wxPGEditorDialogAdapter* GetEditorDialog() const override {
        return new CustomModelDialogAdapter(m_model);
    }
protected:
    CustomModel *m_model;
};


void CustomModel::AddTypeProperties(wxPropertyGridInterface *grid) {
    wxPGProperty *p = grid->Append(new CustomModelProperty(this, "Model Data", "CustomData", CLICK_TO_EDIT));
    grid->LimitPropertyEditing(p);
    p = grid->Append(new wxImageFileProperty("Background Image",
                                             "CustomBkgImage",
                                             custom_background));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Image files|*.png;*.bmp;*.jpg;*.gif|All files (*.*)|*.*");
}
int CustomModel::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("CustomData" == event.GetPropertyName()) {
        if (grid->GetPropertyByName("CustomBkgImage")->GetValue() != custom_background)
        {
            grid->GetPropertyByName("CustomBkgImage")->SetValue(wxVariant(custom_background));
        }
        return 3 | 0x0008;
    }
    else if ("CustomBkgImage" == event.GetPropertyName()) {
        custom_background = event.GetValue().GetString();
        ModelXml->DeleteAttribute("CustomBkgImage");
        ModelXml->AddAttribute("CustomBkgImage", custom_background);
        SetFromXml(ModelXml, zeroBased);
        return 3;
    }
    return Model::OnPropertyGridChange(grid, event);
}


int CustomModel::GetStrandLength(int strand) const {
    return Nodes.size();
}

int CustomModel::MapToNodeIndex(int strand, int node) const {
    return node;
}


void CustomModel::InitModel() {
    std::string customModel = ModelXml->GetAttribute("CustomModel").ToStdString();
    InitCustomMatrix(customModel);
    CopyBufCoord2ScreenCoord();
    custom_background = ModelXml->GetAttribute("CustomBkgImage").ToStdString();
}

void CustomModel::SetCustomWidth(long w) {
    ModelXml->DeleteAttribute("parm1");
    ModelXml->AddAttribute("parm1", wxString::Format("%ld", w));
    SetFromXml(ModelXml, zeroBased);
}
void CustomModel::SetCustomHeight(long h) {
    ModelXml->DeleteAttribute("parm2");
    ModelXml->AddAttribute("parm2", wxString::Format("%ld", h));
    SetFromXml(ModelXml, zeroBased);
}

std::string CustomModel::GetCustomData() const {
    return ModelXml->GetAttribute("CustomModel").ToStdString();
}
void CustomModel::SetCustomData(const std::string &data) {
    ModelXml->DeleteAttribute("CustomModel");
    ModelXml->AddAttribute("CustomModel", data);
    SetFromXml(ModelXml, zeroBased);
}

void CustomModel::SetCustomBackground(std::string background)
{
    custom_background = background;
    ModelXml->DeleteAttribute("CustomBkgImage");
    ModelXml->AddAttribute("CustomBkgImage", custom_background);
}

long CustomModel::GetCustomLightness() const
{
    long lightness;
    ModelXml->GetAttribute("CustomBkgLightness", "0").ToLong(&lightness);
    return lightness;
}

void CustomModel::SetCustomLightness(long lightness)
{
    ModelXml->DeleteAttribute("CustomBkgLightness");
    ModelXml->AddAttribute("CustomBkgLightness", wxString::Format("%ld", lightness));
    SetFromXml(ModelXml, zeroBased);
}

void CustomModel::SetStringStartChannels(bool zeroBased, int NumberOfStrings, int StartChannel, int ChannelsPerString) {
    std::string customModel = ModelXml->GetAttribute("CustomModel").ToStdString();
    int maxval=GetCustomMaxChannel(customModel);
    // fix NumberOfStrings
    if (SingleNode) {
        NumberOfStrings=maxval;
    } else {
        ChannelsPerString=maxval*GetNodeChannelCount(StringType);
    }
    Model::SetStringStartChannels(zeroBased, NumberOfStrings, StartChannel, ChannelsPerString);
}

int CustomModel::NodesPerString()
{
    return GetChanCount() / GetChanCountPerNode();
}

inline void split(std::string frag, char splitBy, std::vector<std::string>& tokens)
{
    // Loop infinitely - break is internal.
    while(true) {
        size_t splitAt = frag.find(splitBy);
        // If we didn't find a new split point...
        if (splitAt == std::string::npos) {
            tokens.push_back(frag);
            break;
        }
        tokens.push_back(frag.substr(0, splitAt));
        frag.erase(0, splitAt + 1);
    }
}

int CustomModel::GetCustomMaxChannel(const std::string& customModel) const
{
    int maxval = 0;

    std::vector<std::string> rows;
    std::vector<std::string> cols;
    rows.reserve(100);
    cols.reserve(100);
    split(customModel, ';', rows);
    
    for (auto row: rows) {
        cols.clear();
        split(row, ',', cols);
        for (auto col : cols) {
            if (col != "") {
                try {
                    maxval=std::max(std::stoi(col),maxval);
                } catch (...) {
                    // not a number, treat as 0
                }
            }
        }
    }
    return maxval;
}

void CustomModel::InitCustomMatrix(const std::string& customModel) {
    int width=1;
    std::vector<int> nodemap;

    int cpn = -1;
    std::vector<std::string> rows;
    std::vector<std::string> cols;
    rows.reserve(100);
    cols.reserve(100);
    split(customModel, ';', rows);
    int height=rows.size();

    int row = 0;
    for (auto rv : rows) {
        cols.clear();
        split(rv, ',', cols);
        if (cols.size() > width) width=cols.size();
        int col = 0;
        for (auto value : cols) {
            while (value.length() > 0 && value[0] == ' ') {
                value = value.substr(1);
            }
            long idx = -1;
            if (value != "") {
                try {
                    idx = std::stoi(value);
                } catch (...) {
                    // not a number, treat as 0
                }
            }
            if (idx > 0) {
                // increase nodemap size if necessary
                if (idx > nodemap.size()) {
                    nodemap.resize(idx, -1);
                }
                idx--;  // adjust to 0-based

                // is node already defined in map?
                if (nodemap[idx] < 0) {
                    // unmapped - so add a node
                    nodemap[idx]=Nodes.size();
                    SetNodeCount(1,0,rgbOrder);  // this creates a node of the correct class
                    Nodes.back()->StringNum=idx;
                    if (cpn == -1) {
                        cpn = GetChanCountPerNode();
                    }
                    Nodes.back()->ActChan=stringStartChan[0] + idx * cpn;
                    if (idx < nodeNames.size() && nodeNames[idx] != "") {
                        Nodes.back()->SetName(nodeNames[idx]);
                    } else {
                        Nodes.back()->SetName("Node "  + std::to_string(idx + 1));
                    }

                    Nodes.back()->AddBufCoord(col,height - row - 1);
                } else {
                    // mapped - so add a coord to existing node
                    Nodes[nodemap[idx]]->AddBufCoord(col,height - row - 1);
                }
            }
            col++;
        }
        row++;
    }
    for (int x = 0; x < Nodes.size(); x++) {
        for (int y = x+1; y < Nodes.size(); y++) {
            if (Nodes[y]->StringNum < Nodes[x]->StringNum) {
                Nodes[x].swap(Nodes[y]);
            }
        }
    }
    for (int x = 0; x < Nodes.size(); x++) {
        if (Nodes[x]->GetName() == "") {
            Nodes[x]->SetName(GetNodeName(Nodes[x]->StringNum));
        }
    }

    SetBufferSize(height,width);
}
std::string CustomModel::GetNodeName(size_t x, bool def) const {
    if (x < Nodes.size()) {
        return Nodes[x]->GetName();
    }
    if (def) {
        return wxString::Format("Node %d", (x + 1)).ToStdString();
    }
    return "";
}

std::list<std::string> CustomModel::CheckModelSettings()
{
    std::list<std::string> res;

    // check for no nodes
    if (GetNodeCount() == 0)
    {
        res.push_back(wxString::Format("    ERR: Custom model '%s' has no nodes defined.", (const char *)GetName().c_str()).ToStdString());
    }

    // check for node gaps
    int maxn = 0;
    for (int ii = 0; ii < GetNodeCount(); ii++)
    {
        int nn = GetNodeStringNumber(ii);
        if (nn > maxn) maxn = nn;
    }
    maxn++;
    int chssize = (maxn + 1) * sizeof(int);
    //logger_base.debug("    CheckSequence: Checking custom model %d nodes", maxn);
    int* chs = (int*)malloc(chssize);
    if (chs == nullptr)
    {
        res.push_back(wxString::Format("    WARN: Could not check Custom model '%s' for missing nodes. Error allocating memory for %d nodes.", (const char *)GetName().c_str(), maxn).ToStdString());
    }
    else
    {
        memset(chs, 0x00, chssize);

        for (int ii = 0; ii < GetNodeCount(); ii++)
        {
            int nn = GetNodeStringNumber(ii);
            chs[nn + 1]++;
        }

        for (int ii = 1; ii <= maxn; ii++)
        {
            if (chs[ii] == 0)
            {
                res.push_back(wxString::Format("    WARN: Custom model '%s' missing node %d.", (const char *)GetName().c_str(), ii).ToStdString());
            }
        }

        int multinodecount = 0;
        for (int ii = 0; ii < GetNodeCount(); ii++)
        {
            std::vector<wxPoint> pts;
            GetNodeCoords(ii, pts);
            if (pts.size() > 1)
            {
                multinodecount++;
            }
        }

        // >0% but less than 10% multi-nodes ... these may be accidental duplicates
        if (multinodecount > 0 && multinodecount < 0.1 * maxn)
        {
            for (int ii = 0; ii < GetNodeCount(); ii++)
            {
                std::vector<wxPoint> pts;
                GetNodeCoords(ii, pts);
                if (pts.size() > 1)
                {
                    res.push_back(wxString::Format("    WARN: Custom model '%s' %s node has %d instances but multi instance nodes are rare in this model so this may be unintended.", (const char *)GetName().c_str(), Ordinal(ii + 1), (int)pts.size()).ToStdString());
                }
            }
        }

        free(chs);
    }

    return res;
}

std::string CustomModel::ChannelLayoutHtml(OutputManager* outputManager) {
    size_t NodeCount=GetNodeCount();
    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    std::string direction="n/a";

    long sc;
    Output* o = outputManager->GetOutput(this->GetFirstChannel(), sc);

    std::string html = "<html><body><table border=0>";
    html+="<tr><td>Name:</td><td>"+name+"</td></tr>";
    html+="<tr><td>Display As:</td><td>"+DisplayAs+"</td></tr>";
    html+="<tr><td>String Type:</td><td>"+StringType+"</td></tr>";
    html+="<tr><td>Start Corner:</td><td>"+direction+"</td></tr>";
    html+=wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>",(int)NodeCount);
    html+=wxString::Format("<tr><td>Height:</td><td>%d</td></tr>",BufferHt);
    if (o != nullptr)
        html += wxString::Format("<tr><td>Controller:</td><td>%s:%s</td></tr>", (o->GetIP() != "" ? o->GetIP() : o->GetCommPort()), o->GetDescription());
    if (controller_connection != "" && wxString(controller_connection).Contains(":"))
    {
        wxArrayString cc = wxSplit(controller_connection, ':');
        html += wxString::Format("<tr><td>Pixel protocol:</td><td>%s</td></tr>", cc[0]);
        html += wxString::Format("<tr><td>Controller Connection:</td><td>%s</td></tr>", cc[1]);
    }
    html+="</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    std::string data = GetCustomData();
    if (data == "") {
            html+="<tr><td>No custom data</td></tr>";
    }

    wxArrayString rows=wxSplit(data, ';');
    for(size_t row=0; row < rows.size(); row++)
    {
        html+="<tr>";
        wxArrayString cols = wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++)
        {
            wxString value=cols[col];
            if (!value.IsEmpty() && value != "0")
            {
                wxString bgcolor = "#ADD8E6"; //"#90EE90"
                html+=wxString::Format("<td bgcolor='"+bgcolor+"'>n%s</td>",value);
            }
            else
            {
                html+="<td>&nbsp&nbsp&nbsp</td>";
            }
        }
        html+="</tr>";
    }
    html+="</table></body></html>";
    return html;
}

void CustomModel::ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    if (!wxString(filename).Lower().EndsWith("xmodel"))
    {
        return ImportLORModel(filename, xlights, min_x, max_x, min_y, max_y);
    }

    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        wxXmlNode* root = doc.GetRoot();

        if (root->GetName() == "custommodel")
        {
            wxString name = root->GetAttribute("name");
            wxString cm = root->GetAttribute("CustomModel");
            wxString p1 = root->GetAttribute("parm1");
            wxString p2 = root->GetAttribute("parm2");
            wxString st = root->GetAttribute("StringType");
            wxString ps = root->GetAttribute("PixelSize");
            wxString t = root->GetAttribute("Transparency");
            wxString mb = root->GetAttribute("ModelBrightness");
            wxString a = root->GetAttribute("Antialias");
            wxString sn = root->GetAttribute("StrandNames");
            wxString nn = root->GetAttribute("NodeNames");
            wxString v = root->GetAttribute("SourceVersion");
            wxString pc = root->GetAttribute("PixelCount");
            wxString pt = root->GetAttribute("PixelType");
            wxString psp = root->GetAttribute("PixelSpacing");

            // Add any model version conversion logic here
            // Source version will be the program version that created the custom model

            SetProperty("CustomModel", cm);
            SetProperty("parm1", p1);
            SetProperty("parm2", p2);
            SetProperty("StringType", st);
            SetProperty("PixelSize", ps);
            SetProperty("Transparency", t);
            SetProperty("ModelBrightness", mb);
            SetProperty("Antialias", a);
            SetProperty("StrandNames", sn);
            SetProperty("NodeNames", nn);
            SetProperty("PixelCount", pc);
            SetProperty("PixelType", pt);
            SetProperty("PixelSpacing", psp);
            wxString newname = xlights->AllModels.GenerateModelName(name.ToStdString());
            SetProperty("name", newname, true);

            for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
            {
                if (n->GetName() == "faceInfo")
                {
                    AddFace(n);
                }
                else if (n->GetName() == "stateInfo")
                {
                    AddState(n);
                }
                else if (n->GetName() == "subModel")
                {
                    AddSubmodel(n);
                }
            }

            xlights->MarkEffectsFileDirty(true);
        }
        else
        {
            wxMessageBox("Failure loading custom model file.");
        }
    }
    else
    {
        wxMessageBox("Failure loading custom model file.");
    }
}

bool point_compare(const wxPoint first, const wxPoint second)
{
    if (first.x == second.x)
    {
        return first.y < second.y;
    }

    return first.x < second.x;
}

void RemoveDuplicatePixels(std::list<std::list<wxPoint>>& chs)
{
    std::list<wxPoint> flat;
    std::list<wxPoint> duplicates;

    for (auto ch = chs.begin(); ch != chs.end(); ++ch)
    {
        for (auto it = ch->begin(); it != ch->end(); ++it)
        {
            flat.push_back(wxPoint(it->x, it->y));
        }
    }

    flat.sort(point_compare);

    for (auto it = flat.begin(); it != flat.end(); ++it)
    {
        auto it2 = it;
        ++it2;

        if (it2 != flat.end())
        {
            if (it->x == it2->x && it->y == it2->y && 
                (duplicates.size() == 0 || duplicates.back().x != it->x || duplicates.back().y != it->y))
            {
                duplicates.push_back(*it);
            }
        }
    }

    for (auto d = duplicates.begin(); d != duplicates.end(); ++d)
    {
        bool first = true;

        for (auto ch = chs.begin(); ch != chs.end(); ++ch)
        {
            auto it = ch->begin();
            while (it != ch->end())
            {
                if (it->x == d->x && it->y == d->y)
                {
                    if (first)
                    {
                        first = false;
                        ++it;
                    }
                    else
                    {
                        ch->erase(it++);
                    }
                }
                else
                {
                    ++it;
                }
            }
        }
    }
}

bool HasDuplicates(float divisor, std::list<std::list<wxPoint>> chs)
{
    std::list<wxPoint> scaled;

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Checking for duplicates at scale %f.", divisor);

    for (auto ch = chs.begin(); ch != chs.end(); ++ch)
    {
        for (auto it = ch->begin(); it != ch->end(); ++it)
        {
            scaled.push_back(wxPoint((float)it->x * divisor, (float)it->y * divisor));
        }
    }

    scaled.sort(point_compare);

    for (auto it = scaled.begin(); it != scaled.end(); ++it)
    {
        auto it2 = it;
        ++it2;

        if (it2 != scaled.end())
        {
            if (it->x == it2->x && it->y == it2->y) return true;
        }
    }

    return false;
}

void CustomModel::ImportLORModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxXmlDocument doc(filename);

    if (doc.IsOk())
    {
        logger_base.debug("Loading LOR model %s.", (const char *)filename.c_str());

        wxXmlNode* root = doc.GetRoot();

        std::list<std::list<wxPoint>> chs;

        for (wxXmlNode* n1 = root->GetChildren(); n1 != nullptr; n1 = n1->GetNext())
        {
            if (n1->GetName() == "DrawObjects")
            {
                for (wxXmlNode* n2 = n1->GetChildren(); n2 != nullptr; n2 = n2->GetNext())
                {
                    if (n2->GetName() == "DrawObject")
                    {
                        for (wxXmlNode* n3 = n2->GetChildren(); n3 != nullptr; n3 = n3->GetNext())
                        {
                            if (n3->GetName() == "DrawPoints")
                            {
                                std::list<wxPoint> points;
                                for (wxXmlNode* n4 = n3->GetChildren(); n4 != nullptr; n4 = n4->GetNext())
                                {
                                    if (n4->GetName() == "DrawPoint")
                                    {
                                        points.push_back(wxPoint(wxAtoi(n4->GetAttribute("X", "-5")) / 5, wxAtoi(n4->GetAttribute("Y", "-1")) / 5));
                                    }
                                }
                                chs.push_back(points);
                            }
                        }
                    }
                }
            }
        }

        wxFileName fn(filename);
        wxString newname = xlights->AllModels.GenerateModelName(fn.GetName().ToStdString());
        SetProperty("name", newname, true);

        xlights->MarkEffectsFileDirty(true);

        if (chs.size() == 0)
        {
            logger_base.error("No model data found.");
            wxMessageBox("Unable to import model data.");
            return;
        }

        int minx = 999999999;
        int maxx = -1;
        int miny = 999999999;
        int maxy = -1;

        for (auto ch = chs.begin(); ch != chs.end(); ++ch)
        {
            for (auto it = ch->begin(); it != ch->end(); ++it)
            {
                if (it->x >= 0)
                {
                    if (it->x < minx) minx = it->x;
                    if (it->x > maxx) maxx = it->x;
                }
                if (it->y >= 0)
                {
                    if (it->y < miny) miny = it->y;
                    if (it->y > maxy) maxy = it->y;
                }
            }
        }

        for (auto ch = chs.begin(); ch != chs.end(); ++ch)
        {
            for (auto it = ch->begin(); it != ch->end(); ++it)
            {
                it->x = (it->x - minx);
                it->y = (it->y - miny);
            }
        }

        maxx -= minx;
        maxy -= miny;

        float divisor = 0.1f;
        if (HasDuplicates(1.0, chs))
        {
            logger_base.warn("This model is not going to import correctly as some pixels overlap.");
            wxMessageBox("WARNING: This model is not going to import correctly as one or more pixels overlap.");

            RemoveDuplicatePixels(chs);
        }

        while (HasDuplicates(divisor, chs))
            {
                divisor += 0.1f;

                if (divisor >= 1.0f) break;
            }

            divisor -= 0.1f + 0.01f;

            while (HasDuplicates(divisor, chs))
            {
                divisor += 0.01f;

                if (divisor >= 1.0f) break;
            }

        maxx = ((float)maxx * divisor) + 1;
        maxy = ((float)maxy * divisor) + 1;

        logger_base.debug("Divisor chosen %f. Model dimensions %d,%d", divisor, maxx+1, maxy+1);

        SetProperty("parm1", wxString::Format("%i", maxx));
        SetProperty("parm2", wxString::Format("%i", maxy));

        int* data = (int*)malloc(maxx *  maxy * sizeof(int));
        memset(data, 0x00, maxx *  maxy * sizeof(int));

        int c = 1;

        for (auto ch = chs.begin(); ch != chs.end(); ++ch)
        {
            for (auto it = ch->begin(); it != ch->end(); ++it)
            {
                int x = (float)it->x * divisor;
                int y = (float)it->y * divisor;

                wxASSERT(x >= 0 && x < maxx);
                wxASSERT(y >= 0 && y < maxy);

                data[y*maxx + x] = c;
            }
            c++;
        }

        std::string cm = "";
        for (int y = 0; y < maxy; ++y)
        {
            for (int x = 0; x < maxx; ++x)
            {
                if (data[y * maxx + x] != 0)
                {
                    cm += wxString::Format("%i", data[y * maxx + x]);
                }
                if (x != maxx - 1) cm += ",";
            }

            if (y != maxy - 1) cm += ";";
        }
        free(data);

        SetProperty("CustomModel", cm);
        logger_base.debug("Model import done.");
    }
    else
    {
        wxMessageBox("Failure loading LOR model file.");
    }
}

void CustomModel::ExportXlightsModel()
{
    wxString name = ModelXml->GetAttribute("name");
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets after new file is written
    wxString filename = wxFileSelector(_("Choose output file"), wxEmptyString, name, wxEmptyString, "Custom Model files (*.xmodel)|*.xmodel", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (filename.IsEmpty()) return;
    wxFile f(filename);
    //    bool isnew = !wxFile::Exists(filename);
    if (!f.Create(filename, true) || !f.IsOpened()) retmsg(wxString::Format("Unable to create file %s. Error %d\n", filename, f.GetLastError()));
    wxString cm = ModelXml->GetAttribute("CustomModel");
    wxString p1 = ModelXml->GetAttribute("parm1");
    wxString p2 = ModelXml->GetAttribute("parm2");
    wxString st = ModelXml->GetAttribute("StringType");
    wxString ps = ModelXml->GetAttribute("PixelSize");
    wxString t = ModelXml->GetAttribute("Transparency");
    wxString mb = ModelXml->GetAttribute("ModelBrightness");
    wxString a = ModelXml->GetAttribute("Antialias");
    wxString sn = ModelXml->GetAttribute("StrandNames");
    wxString nn = ModelXml->GetAttribute("NodeNames");
    wxString v = xlights_version_string;
    f.Write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<custommodel \n");
    f.Write(wxString::Format("name=\"%s\" ", name));
    f.Write(wxString::Format("parm1=\"%s\" ", p1));
    f.Write(wxString::Format("parm2=\"%s\" ", p2));
    f.Write(wxString::Format("StringType=\"%s\" ", st));
    f.Write(wxString::Format("Transparency=\"%s\" ", t));
    f.Write(wxString::Format("PixelSize=\"%s\" ", ps));
    f.Write(wxString::Format("ModelBrightness=\"%s\" ", mb));
    f.Write(wxString::Format("Antialias=\"%s\" ", a));
    f.Write(wxString::Format("StrandNames=\"%s\" ", sn));
    f.Write(wxString::Format("NodeNames=\"%s\" ", nn));
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
