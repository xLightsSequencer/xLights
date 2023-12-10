/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xLightsMain.h"
#include "ShaderDownloadDialog.h"

//(*InternalHeaders(ShaderDownloadDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/wx.h>
#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include <wx/progdlg.h>
#include <wx/dir.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include "../xSchedule/wxJSON/jsonreader.h"

#include "CachedFileDownloader.h"
#include "UtilFunctions.h"
#include "ExternalHooks.h"
#include <log4cpp/Category.hh>

#include "utils/Curl.h"
#undef min
#undef max


class MShader
{
    std::string _webpage = "";
    std::string _download = "";

public:
    static std::string __downloadPattern;
    static std::string __webpagePattern;
    std::string _id = "";
    std::string _name = "";
    std::string _fsFile = "";
    int _rating = 0;
    std::string _imageURL = "";
    std::string _imageFile = "";
    std::string _comment = "";

    std::string GetDownload()
    {
        if (__downloadPattern != "" && _id != "") {
            wxString s(__downloadPattern);
            s.Replace("{id}", _id);
            return s.ToStdString();
        }
        return _download;
    }

    std::string GetWebpage()
    {
        if (__webpagePattern != "" && _id != "") {
            wxString s(__webpagePattern);
            s.Replace("{id}", _id);
            return s.ToStdString();
        }
        return _webpage;
    }

    MShader(wxXmlNode* n)
    {
        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext()) {
            wxString nn = l->GetName().Lower().ToStdString();
            if (nn == "name") {
                _name = l->GetNodeContent().ToStdString();
            }
            else if (nn == "link") {
                _webpage = l->GetNodeContent().ToStdString();
            }
            else if (nn == "download") {
                _download = l->GetNodeContent().ToStdString();
            }
            else if (nn == "image") {
                _imageURL = l->GetNodeContent().ToStdString();
            }
            else if (nn == "id") {
                _id = l->GetNodeContent().ToStdString();
            }
            else if (nn == "rating") {
                _rating = wxAtoi(l->GetNodeContent().ToStdString());
            }
            else if (nn == "comment") {
                _comment = l->GetNodeContent().ToStdString();
            }
            else {
                wxASSERT(false);
            }
        }
    }

    virtual ~MShader()
    {
    }

    std::string PadTitle(std::string t) const
    {
        std::string res = t;
        while (res.size() < 18) res += " ";
        return res;
    }

    std::string GetDescription() const
    {
        std::string desc;

        if (_name != "") {
            desc += PadTitle("Name:") + _name + "\n";
        }
        if (_rating != 0) {
            desc += PadTitle(wxString::Format("Rating: %d/5", _rating));
        }

        return desc;
    }

    void DownloadImages()
    {
        if (_imageURL != "") {
            _imageFile = ShaderDownloadDialog::GetCache().GetFile(wxURI(_imageURL), CACHEFOR::CACHETIME_LONG, "jpg");
        }
    }

    std::string ExtractFSFromZip(const std::string& zipFile)
    {
        std::string fn;

        wxFileInputStream fin(zipFile);
        wxZipInputStream zin(fin);
        wxZipEntry* ent = zin.GetNextEntry();
        while (ent != nullptr) {
            if (wxFileName(ent->GetName()).GetExt().Lower() == "fs") {
                fn = xLightsFrame::CurrentDir + "/Shaders/" + ent->GetName();
                wxFileOutputStream fout(fn);
                zin.Read(fout);
                return fn;
            }
            ent = zin.GetNextEntry();
        }

        return "";
    }

    void DownloadFS()
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        if (xLightsFrame::CurrentDir == "") {
            wxMessageBox("Show folder invalid. Shader download aborted.");
        }

        auto download = GetDownload();

        if (download != "") {
            // make sure the shaders folder is there
            wxMkDir(xLightsFrame::CurrentDir + "/Shaders", wxS_DIR_DEFAULT);
            std::string fn = xLightsFrame::CurrentDir + "/Shaders/" + _name + ".fs";
            if (Curl::HTTPSGetFile(download, fn)) {
                wxFile f;
                if (f.Open(fn)) {
                    char token[3];
                    memset(token, 0x00, sizeof(token));
                    f.Read(token, sizeof(token) - 1);
                    f.Close();
                    if (strcmp(token, "PK") == 0) {
                        wxRenameFile(fn, xLightsFrame::CurrentDir + "/Shaders/" + _name + ".zip", true);
                        fn = ExtractFSFromZip(xLightsFrame::CurrentDir + "/Shaders/" + _name + ".zip");
                    }
                    else if (strcmp(token, "<!") == 0) {
                        wxRemoveFile(fn);
                        fn = "";
                        logger_base.debug("Shader file download failed and returned a non fs file.");
                    }
                    else if (strcmp(token, "{\\\"")) {
                        wxString newFile = xLightsFrame::CurrentDir + "/Shaders/" + _name + ".json";
                        wxRenameFile(fn, newFile, true);

                        wxFile ff(newFile);
                        if (!ff.IsOpened()) {
                            wxRemoveFile(fn);
                            fn = "";
                            logger_base.debug("Shader file download failed load fs file.");
                        }
                        else {
                            wxString json;
                            ff.ReadAll(&json);
                            ff.Close();

                            wxJSONReader reader;
                            wxJSONValue root;
                            reader.Parse(json, &root);
                            auto shader = root["rawFragmentSource"].AsString();

                            wxFile fff(fn, wxFile::OpenMode::write);
                            if (fff.IsOpened()) {
                                fff.Write(shader);
                                fff.Close();
                            }
                            else {
                                logger_base.debug("Shader file download failed load to create fs file.");
                            }
                        }
                    }
                    else {
                        wxRemoveFile(fn);
                        fn = "";
                        logger_base.debug("Shader file download failed and returned a non fs file.");
                    }
                }

                _fsFile = fn;
            }
        }
    }
};

std::string MShader::__downloadPattern;
std::string MShader::__webpagePattern;

class ShaderTreeItemData : public wxTreeItemData
{
public:
    ShaderTreeItemData(MShader* shader) : _shader(shader) { }
    MShader* GetShader() const { return _shader; }

private:
    MShader* _shader;
};

//(*IdInit(ShaderDownloadDialog)
const long ShaderDownloadDialog::ID_TREECTRL1 = wxNewId();
const long ShaderDownloadDialog::ID_STATICTEXT1 = wxNewId();
const long ShaderDownloadDialog::ID_LISTVIEW1 = wxNewId();
const long ShaderDownloadDialog::ID_PANEL3 = wxNewId();
const long ShaderDownloadDialog::ID_STATICBITMAP1 = wxNewId();
const long ShaderDownloadDialog::ID_TEXTCTRL1 = wxNewId();
const long ShaderDownloadDialog::ID_STATICTEXT4 = wxNewId();
const long ShaderDownloadDialog::ID_HYPERLINKCTRL4 = wxNewId();
const long ShaderDownloadDialog::ID_BUTTON4 = wxNewId();
const long ShaderDownloadDialog::ID_PANEL1 = wxNewId();
const long ShaderDownloadDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ShaderDownloadDialog,wxDialog)
	//(*EventTable(ShaderDownloadDialog)
	//*)
END_EVENT_TABLE()

ShaderDownloadDialog::ShaderDownloadDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	//(*Initialize(ShaderDownloadDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(800,600));
	SetMinSize(wxSize(800,400));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetSashGravity(0.5);
	Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	TreeCtrl_Navigator = new wxTreeCtrl(Panel3, ID_TREECTRL1, wxDefaultPosition, wxSize(200,-1), wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT|wxTR_ROW_LINES|wxTR_SINGLE|wxTR_DEFAULT_STYLE|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_TREECTRL1"));
	FlexGridSizer2->Add(TreeCtrl_Navigator, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(Panel3, ID_STATICTEXT1, _("Shader sites"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	ListView_Sites = new wxListView(Panel3, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_ALIGN_LEFT|wxLC_NO_HEADER|wxLC_NO_SORT_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer2->Add(ListView_Sites, 1, wxALL|wxEXPAND, 5);
	Panel3->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel3);
	FlexGridSizer2->SetSizeHints(Panel3);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	StaticBitmap_VendorImage = new wxStaticBitmap(Panel1, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(300,300), wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	StaticBitmap_VendorImage->SetMinSize(wxSize(256,128));
	FlexGridSizer3->Add(StaticBitmap_VendorImage, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Details = new wxTextCtrl(Panel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer3->Add(TextCtrl_Details, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer4->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HyperlinkCtrl_Website = new wxHyperlinkCtrl(Panel1, ID_HYPERLINKCTRL4, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxNO_BORDER, _T("ID_HYPERLINKCTRL4"));
	FlexGridSizer4->Add(HyperlinkCtrl_Website, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	Button_Download = new wxButton(Panel1, ID_BUTTON4, _("Download"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer3->Add(Button_Download, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel1->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel1);
	FlexGridSizer3->SetSizeHints(Panel1);
	SplitterWindow1->SplitVertically(Panel3, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&ShaderDownloadDialog::OnTreeCtrl_NavigatorItemActivated);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&ShaderDownloadDialog::OnTreeCtrl_NavigatorSelectionChanged);
	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&ShaderDownloadDialog::OnListView_SitesItemActivated);
	Connect(ID_HYPERLINKCTRL4,wxEVT_COMMAND_HYPERLINK,(wxObjectEventFunction)&ShaderDownloadDialog::OnHyperlinkCtrl_WebsiteClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ShaderDownloadDialog::OnButton_DownloadClick);
	//*)

    SetSize(800, 600);

    ListView_Sites->AppendColumn("", wxListColumnFormat::wxLIST_FORMAT_LEFT);
    ListView_Sites->InsertItem(0, "www.interactiveshaderformat.com/");
    ListView_Sites->InsertItem(0, "www.shadertoy.com/");
    ListView_Sites->InsertItem(0, "glslsandbox.com/");
    ListView_Sites->SetColumnWidth(0, wxLIST_AUTOSIZE);

    HyperlinkCtrl_Website->SetNormalColour(CyanOrBlue());
    PopulateShaderPanel((MShader*)nullptr);

    ValidateWindow();
}

bool ShaderDownloadDialog::DlgInit(wxProgressDialog* prog, int low, int high)
{
    if (LoadTree(prog, low, high))
    {
        ValidateWindow();
        return true;
    }

    return false;
}

wxXmlDocument* ShaderDownloadDialog::GetXMLFromURL(wxURI url, std::string& filename, wxProgressDialog* prog, int low, int high) const
{
    filename = "";
    wxFileName fn = wxFileName(ShaderDownloadDialog::GetCache().GetFile(url, CACHEFOR::CACHETIME_SESSION, "", prog, low, high));
    if (FileExists(fn))
    {
        filename = fn.GetFullPath();
        return new wxXmlDocument(fn.GetFullPath());
    }

    return nullptr;
}

bool ShaderDownloadDialog::LoadTree(wxProgressDialog* prog, int low, int high)
{
    const std::string shaderlink = "https://raw.githubusercontent.com/smeighan/xLights/master/download/shaders.xml";

    bool rc = true;
    std::string filename;
    wxXmlDocument* vd = GetXMLFromURL(wxURI(shaderlink), filename, prog, low, high);
    if (vd != nullptr && vd->IsOk())
    {
        wxXmlNode* root = vd->GetRoot();

        for (auto v = root->GetChildren(); v != nullptr; v = v->GetNext())
        {
            if (v->GetName().Lower() == "shader")
            {
                _shaders.push_back(new MShader(v));
            }
            else if (v->GetName().Lower() == "downloadmaster")
            {
                MShader::__downloadPattern = v->GetNodeContent().ToStdString();
            }
            else if (v->GetName().Lower() == "linkmaster")
            {
                MShader::__webpagePattern = v->GetNodeContent().ToStdString();
            }
        }
    }
    if (vd != nullptr)
    {
        delete vd;
    }

    TreeCtrl_Navigator->Freeze();
    TreeCtrl_Navigator->DeleteAllItems();
    wxTreeItemId root = TreeCtrl_Navigator->AddRoot("Shaders");
    for (const auto& it : _shaders)
    {
        TreeCtrl_Navigator->AppendItem(root, it->_name, -1, -1, new ShaderTreeItemData(it));
    }
    TreeCtrl_Navigator->SortChildren(root);
    if (_shaders.size() > 0)
    {
        wxTreeItemIdValue cookie;
        TreeCtrl_Navigator->EnsureVisible(TreeCtrl_Navigator->GetFirstChild(root, cookie));
    }
    else
    {
        DisplayError("Unable to retrieve any shader information", this);
        rc = false;
    }
    TreeCtrl_Navigator->Thaw();

    return rc;
}

ShaderDownloadDialog::~ShaderDownloadDialog()
{
	//(*Destroy(ShaderDownloadDialog)
	//*)

    GetCache().Save();

    for (const auto& it : _shaders)
    {
        delete it;
    }
}

void ShaderDownloadDialog::OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event)
{
    DoDownload(event.GetItem());
    EndDialog(wxID_OK);
}

void ShaderDownloadDialog::OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event)
{
    static bool busy = false;

    if (busy)
    {
        return;
    }

    // Because this code triggers a web download this function can be re-entered and this is not good
    busy = true;

    wxTreeItemId startid = TreeCtrl_Navigator->GetSelection();

    SetCursor(wxCURSOR_WAIT);

    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(startid);

        if (tid != nullptr)
        {
            if (tid == TreeCtrl_Navigator->GetRootItem())
            {
                PopulateShaderPanel(nullptr);
            }
            else
            {
                PopulateShaderPanel(((ShaderTreeItemData*)tid)->GetShader());
            }
            TreeCtrl_Navigator->SetFocus();
        }
    }
    else
    {
        PopulateShaderPanel(nullptr);
    }

    ValidateWindow();

    SetCursor(wxCURSOR_DEFAULT);

    busy = false;

    if (startid != TreeCtrl_Navigator->GetSelection())
    {
        // selection changed while we were processing so lets try again
        wxPostEvent(this, event);
    }
}

void ShaderDownloadDialog::OnHyperlinkCtrl_WebsiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_Website->GetURL());
}

void ShaderDownloadDialog::ValidateWindow()
{
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr && tid != TreeCtrl_Navigator->GetRootItem())
        {
            Button_Download->Enable();
        }
        else
        {
            Button_Download->Disable();
        }
    }
    else
    {
        Button_Download->Disable();
    }
}

void ShaderDownloadDialog::PopulateShaderPanel(MShader* shader)
{
    wxLogNull logNo; //kludge: avoid error message when trying to load image

    if (shader == nullptr)
    {
        StaticBitmap_VendorImage->Hide();
        TextCtrl_Details->Hide();
        Button_Download->Hide();
        StaticText2->Hide();
        HyperlinkCtrl_Website->Hide();
        return;
    }

    shader->DownloadImages();
    if (FileExists(shader->_imageFile))
    {
        if (_shaderImage.LoadFile(shader->_imageFile) && _shaderImage.IsOk())
        {
            StaticBitmap_VendorImage->Show();
            LoadImage(StaticBitmap_VendorImage, &_shaderImage);
        }
        else
        {
            wxRemoveFile(shader->_imageFile);
            shader->_imageFile = "";
            StaticBitmap_VendorImage->Hide();
        }
    }
    else
    {
        StaticBitmap_VendorImage->Hide();
    }

    if (shader->GetDescription() != "")
    {
        TextCtrl_Details->Show();
        TextCtrl_Details->SetValue(shader->GetDescription());
    }
    else
    {
        TextCtrl_Details->Hide();
    }

    if (shader->GetWebpage() != "")
    {
        StaticText2->Show();
        HyperlinkCtrl_Website->Show();
        HyperlinkCtrl_Website->SetURL(shader->GetWebpage());
        HyperlinkCtrl_Website->SetLabel(shader->GetWebpage());
    }
    else
    {
        StaticText2->Hide();
        HyperlinkCtrl_Website->Hide();
    }

    if (shader->GetDownload() != "")
    {
        Button_Download->Show();
    }
    else
    {
        Button_Download->Hide();
    }

    Panel1->Layout();
}

void ShaderDownloadDialog::LoadImage(wxStaticBitmap* sb, wxImage* image) const
{
    if (image->GetWidth() == 0 || image->GetHeight() == 0) return;

    wxImage disp = image->Copy();

    float x = (float)sb->GetSize().GetWidth() / (float)disp.GetWidth();
    float y = (float)sb->GetSize().GetHeight() / (float)disp.GetHeight();
    float scale = std::min(x, y);

    sb->SetBitmap(disp.Rescale((float)disp.GetWidth() * scale, (float)disp.GetHeight() * scale));
}

void ShaderDownloadDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void ShaderDownloadDialog::OnResize(wxSizeEvent& event)
{
    wxDialog::OnSize(event);

    if (StaticBitmap_VendorImage->IsShown() && _shaderImage.IsOk())
    {
        if (_shaderImage.GetSize() != StaticBitmap_VendorImage->GetSize())
        {
            LoadImage(StaticBitmap_VendorImage, &_shaderImage);
        }
    }

    Panel1->Refresh();
}

void ShaderDownloadDialog::OnButton_DownloadClick(wxCommandEvent& event)
{
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        DoDownload(TreeCtrl_Navigator->GetSelection());
    }
    EndDialog(wxID_OK);
}

void ShaderDownloadDialog::OnListView_SitesItemActivated(wxListEvent& event)
{
    auto label = ListView_Sites->GetItemText(event.GetItem());
    ::wxLaunchDefaultBrowser(label);
}

void ShaderDownloadDialog::DoDownload(const wxTreeItemId& treeitem)
{
    wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(treeitem);
    ((ShaderTreeItemData*)tid)->GetShader()->DownloadFS();
    _shaderFile = ((ShaderTreeItemData*)tid)->GetShader()->_fsFile;
}
