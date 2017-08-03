
#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "CustomModel.h"
#include "../CustomModelDialog.h"
#include "../xLightsMain.h"
#include "../xLightsVersion.h"

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
    ModelXml->AddAttribute("parm1", wxString::Format("%d", w));
    SetFromXml(ModelXml, zeroBased);
}
void CustomModel::SetCustomHeight(long h) {
    ModelXml->DeleteAttribute("parm2");
    ModelXml->AddAttribute("parm2", wxString::Format("%d", h));
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

long CustomModel::GetCustomLightness() const
{
    long lightness;
    ModelXml->GetAttribute("CustomBkgLightness", "0").ToLong(&lightness);
    return lightness;
}

void CustomModel::SetCustomLightness(long lightness)
{
    ModelXml->DeleteAttribute("CustomBkgLightness");
    ModelXml->AddAttribute("CustomBkgLightness", wxString::Format("%d", lightness));
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



int CustomModel::GetCustomMaxChannel(const std::string& customModel) {
    wxString value;
    wxArrayString cols;
    long val,maxval=0;
    wxString valstr;

    wxArrayString rows=wxSplit(customModel,';');
    for(size_t row=0; row < rows.size(); row++) {
        cols=wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++) {
            valstr=cols[col];
            if (!valstr.IsEmpty() && valstr != "0") {
                valstr.ToLong(&val);
                maxval=std::max(val,maxval);
            }
        }
    }
    return maxval;
}
void CustomModel::InitCustomMatrix(const std::string& customModel) {
    wxString value;
    wxArrayString cols;
    int width=1;
    std::vector<int> nodemap;

    wxArrayString rows=wxSplit(customModel,';');
    int height=rows.size();
    int cpn = -1;

    for(size_t row=0; row < rows.size(); row++) {
        cols=wxSplit(rows[row],',');
        if (cols.size() > width) width=cols.size();
        for(size_t col=0; col < cols.size(); col++) {
            value=cols[col];
            value.Trim(true);
            value.Trim(false);
            long idx = -1;
            if (!value.IsEmpty() && value != "0") {
                value.ToLong(&idx);
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
                        Nodes.back()->SetName(wxString::Format("Node %d", (idx + 1)).ToStdString());
                    }
                    
                    Nodes.back()->AddBufCoord(col,height - row - 1);
                } else {
                    // mapped - so add a coord to existing node
                    Nodes[nodemap[idx]]->AddBufCoord(col,height - row - 1);
                }
            }
        }
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

std::string CustomModel::ChannelLayoutHtml() {
    size_t NodeCount=GetNodeCount();
    wxString bgcolor;
    std::vector<int> chmap;
    chmap.resize(BufferHt * BufferWi,0);
    std::string direction="n/a";

    std::string html = "<html><body><table border=0>";
    html+="<tr><td>Name:</td><td>"+name+"</td></tr>";
    html+="<tr><td>Display As:</td><td>"+DisplayAs+"</td></tr>";
    html+="<tr><td>String Type:</td><td>"+StringType+"</td></tr>";
    html+="<tr><td>Start Corner:</td><td>"+direction+"</td></tr>";
    html+=wxString::Format("<tr><td>Total nodes:</td><td>%d</td></tr>",NodeCount);
    html+=wxString::Format("<tr><td>Height:</td><td>%d</td></tr>",BufferHt);
    html+="</table><p>Node numbers starting with 1 followed by string number:</p><table border=1>";

    std::string data = GetCustomData();
    if (data == "") {
            html+="<tr><td>No custom data</td></tr>";
    }

    wxArrayString cols;
    wxArrayString rows=wxSplit(data, ';');
    for(size_t row=0; row < rows.size(); row++)
    {
        html+="<tr>";
        cols=wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++)
        {
            wxString value=cols[col];
            if (!value.IsEmpty() && value != "0")
            {
                bgcolor="#ADD8E6"; //"#90EE90"
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
