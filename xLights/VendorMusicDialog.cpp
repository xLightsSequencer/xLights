/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VendorMusicDialog.h"

//(*InternalHeaders(VendorMusicDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <log4cpp/Category.hh>
#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include "CachedFileDownloader.h"
#include "UtilFunctions.h"
#include "xLightsMain.h"

CachedFileDownloader& VendorMusicDialog::GetCache() {
    return CachedFileDownloader::GetDefaultCache();
}

//(*IdInit(VendorMusicDialog)
const long VendorMusicDialog::ID_TREECTRL1 = wxNewId();
const long VendorMusicDialog::ID_TEXTCTRL3 = wxNewId();
const long VendorMusicDialog::ID_BUTTON2 = wxNewId();
const long VendorMusicDialog::ID_PANEL3 = wxNewId();
const long VendorMusicDialog::ID_STATICBITMAP1 = wxNewId();
const long VendorMusicDialog::ID_TEXTCTRL1 = wxNewId();
const long VendorMusicDialog::ID_STATICTEXT8 = wxNewId();
const long VendorMusicDialog::ID_HYPERLINKCTRL4 = wxNewId();
const long VendorMusicDialog::ID_STATICTEXT4 = wxNewId();
const long VendorMusicDialog::ID_HYPERLINKCTRL2 = wxNewId();
const long VendorMusicDialog::ID_PANEL2 = wxNewId();
const long VendorMusicDialog::ID_TEXTCTRL2 = wxNewId();
const long VendorMusicDialog::ID_STATICTEXT1 = wxNewId();
const long VendorMusicDialog::ID_HYPERLINKCTRL1 = wxNewId();
const long VendorMusicDialog::ID_STATICTEXT7 = wxNewId();
const long VendorMusicDialog::ID_HYPERLINKCTRL3 = wxNewId();
const long VendorMusicDialog::ID_STATICTEXT2 = wxNewId();
const long VendorMusicDialog::ID_HYPERLINKCTRL5 = wxNewId();
const long VendorMusicDialog::ID_BUTTON1 = wxNewId();
const long VendorMusicDialog::ID_PANEL4 = wxNewId();
const long VendorMusicDialog::ID_NOTEBOOK1 = wxNewId();
const long VendorMusicDialog::ID_PANEL1 = wxNewId();
const long VendorMusicDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VendorMusicDialog,wxDialog)
	//(*EventTable(VendorMusicDialog)
	//*)
END_EVENT_TABLE()

VendorMusicDialog::VendorMusicDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    //(*Initialize(VendorMusicDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;

    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX | wxMAXIMIZE_BOX, _T("id"));
    SetClientSize(wxSize(800, 600));
    Move(wxDefaultPosition);
    SetMinSize(wxSize(800, 400));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinSize(wxSize(10, 10));
    SplitterWindow1->SetSashGravity(0.5);
    Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(0);
    TreeCtrl_Navigator = new wxTreeCtrl(Panel3, ID_TREECTRL1, wxDefaultPosition, wxSize(200, -1), wxTR_FULL_ROW_HIGHLIGHT | wxTR_HIDE_ROOT | wxTR_ROW_LINES | wxTR_SINGLE | wxTR_DEFAULT_STYLE | wxVSCROLL | wxHSCROLL, wxDefaultValidator, _T("ID_TREECTRL1"));
    FlexGridSizer2->Add(TreeCtrl_Navigator, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer7->AddGrowableCol(0);
    TextCtrl_Search = new wxTextCtrl(Panel3, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL3"));
    FlexGridSizer7->Add(TextCtrl_Search, 1, wxALL | wxEXPAND, 5);
    Button_Search = new wxButton(Panel3, ID_BUTTON2, _("Search"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer7->Add(Button_Search, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer7, 1, wxALL | wxEXPAND, 5);
    Panel3->SetSizer(FlexGridSizer2);
    FlexGridSizer2->Fit(Panel3);
    FlexGridSizer2->SetSizeHints(Panel3);
    Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(0);
    NotebookPanels = new wxNotebook(Panel1, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    PanelVendor = new wxPanel(NotebookPanels, ID_PANEL2, wxPoint(43, 60), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(1);
    StaticBitmap_VendorImage = new wxStaticBitmap(PanelVendor, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(256, 128), wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
    StaticBitmap_VendorImage->SetMinSize(wxSize(256, 128));
    FlexGridSizer4->Add(StaticBitmap_VendorImage, 1, wxALL | wxEXPAND, 5);
    TextCtrl_VendorDetails = new wxTextCtrl(PanelVendor, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer4->Add(TextCtrl_VendorDetails, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(1);
    FlexGridSizer5->AddGrowableRow(0);
    StaticText6 = new wxStaticText(PanelVendor, ID_STATICTEXT8, _("Facebook:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer5->Add(StaticText6, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_Facebook = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL4, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL4"));
    FlexGridSizer5->Add(HyperlinkCtrl_Facebook, 1, wxALL | wxEXPAND, 5);
    StaticText2 = new wxStaticText(PanelVendor, ID_STATICTEXT4, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer5->Add(StaticText2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_Website = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL2, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL2"));
    FlexGridSizer5->Add(HyperlinkCtrl_Website, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL | wxEXPAND, 5);
    PanelVendor->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(PanelVendor);
    FlexGridSizer4->SetSizeHints(PanelVendor);
    Panel_Item = new wxPanel(NotebookPanels, ID_PANEL4, wxPoint(41, 9), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    FlexGridSizer6->AddGrowableRow(0);
    TextCtrl_ModelDetails = new wxTextCtrl(Panel_Item, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer6->Add(TextCtrl_ModelDetails, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText1 = new wxStaticText(Panel_Item, ID_STATICTEXT1, _("Video:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer8->Add(StaticText1, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_VideoLink = new wxHyperlinkCtrl(Panel_Item, ID_HYPERLINKCTRL1, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL1"));
    FlexGridSizer8->Add(HyperlinkCtrl_VideoLink, 1, wxALL | wxEXPAND, 5);
    StaticText5 = new wxStaticText(Panel_Item, ID_STATICTEXT7, _("Web Link:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer8->Add(StaticText5, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_WebLink = new wxHyperlinkCtrl(Panel_Item, ID_HYPERLINKCTRL3, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL3"));
    FlexGridSizer8->Add(HyperlinkCtrl_WebLink, 1, wxALL | wxEXPAND, 5);
    StaticText3 = new wxStaticText(Panel_Item, ID_STATICTEXT2, _("Music Link:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer8->Add(StaticText3, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_MusicLink = new wxHyperlinkCtrl(Panel_Item, ID_HYPERLINKCTRL5, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL5"));
    FlexGridSizer8->Add(HyperlinkCtrl_MusicLink, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer6->Add(FlexGridSizer8, 1, wxALL | wxEXPAND, 5);
    Button_Download = new wxButton(Panel_Item, ID_BUTTON1, _("Download"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer6->Add(Button_Download, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    Panel_Item->SetSizer(FlexGridSizer6);
    FlexGridSizer6->Fit(Panel_Item);
    FlexGridSizer6->SetSizeHints(Panel_Item);
    NotebookPanels->AddPage(PanelVendor, _("Vendor"), false);
    NotebookPanels->AddPage(Panel_Item, _("Item"), false);
    FlexGridSizer3->Add(NotebookPanels, 1, wxALL | wxEXPAND, 5);
    Panel1->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel1);
    FlexGridSizer3->SetSizeHints(Panel1);
    SplitterWindow1->SplitVertically(Panel3, Panel1);
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL | wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_TREECTRL1, wxEVT_COMMAND_TREE_ITEM_ACTIVATED, (wxObjectEventFunction)&VendorMusicDialog::OnTreeCtrl_NavigatorItemActivated);
    Connect(ID_TREECTRL1, wxEVT_COMMAND_TREE_SEL_CHANGED, (wxObjectEventFunction)&VendorMusicDialog::OnTreeCtrl_NavigatorSelectionChanged);
    Connect(ID_TEXTCTRL3, wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&VendorMusicDialog::OnTextCtrl_SearchText);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VendorMusicDialog::OnButton_SearchClick);
    Connect(ID_HYPERLINKCTRL4, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorMusicDialog::OnHyperlinkCtrl_FacebookClick);
    Connect(ID_HYPERLINKCTRL2, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorMusicDialog::OnHyperlinkCtrl_WebsiteClick);
    Connect(ID_HYPERLINKCTRL1, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorMusicDialog::OnHyperlinkCtrl_VideoLinkClick);
    Connect(ID_HYPERLINKCTRL3, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorMusicDialog::OnHyperlinkCtrl_ModelWebLinkClick);
    Connect(ID_HYPERLINKCTRL5, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorMusicDialog::OnHyperlinkCtrl_MusicLinkClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VendorMusicDialog::OnButton_InsertModelClick);
    Connect(ID_NOTEBOOK1, wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&VendorMusicDialog::OnNotebookPanelsPageChanged);
    Connect(wxID_ANY, wxEVT_INIT_DIALOG, (wxObjectEventFunction)&VendorMusicDialog::OnInit);
    //*)

    SetSize(800, 600);

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("File cache size: %d", GetCache().size());

    PopulateSequenceLyricPanel((MSLSequenceLyric*)nullptr);
    PopulateVendorPanel(nullptr);

    Button_Search->SetDefault();

    HyperlinkCtrl_Facebook->SetNormalColour(CyanOrBlue());
    HyperlinkCtrl_Website->SetNormalColour(CyanOrBlue());
    HyperlinkCtrl_VideoLink->SetNormalColour(CyanOrBlue());
    HyperlinkCtrl_WebLink->SetNormalColour(CyanOrBlue());
    HyperlinkCtrl_MusicLink->SetNormalColour(CyanOrBlue());

    ValidateWindow();
}

bool VendorMusicDialog::DlgInit(std::string hash, std::string showFolder)
{
    _showFolder = showFolder;
    if (LoadTree(hash)) {
        ValidateWindow();
        return true;
    }

    return false;
}

wxXmlDocument* VendorMusicDialog::GetXMLFromURL(wxURI url, std::string& filename) const
{
    filename = "";
    wxFileName fn = wxFileName(VendorMusicDialog::GetCache().GetFile(url, CACHEFOR::CACHETIME_SESSION));
    if (fn.Exists()) {
        filename = fn.GetFullPath();
        return new wxXmlDocument(fn.GetFullPath());
    }

    return nullptr;
}

bool VendorMusicDialog::LoadTree(std::string hash)
{
    const std::string vendorlink = "https://nutcracker123.com/xlights/vendors/xlights_vendors.xml";
    const std::string vendorlinkbackup = "https://github.com/smeighan/xLights/raw/master/download/xlights_vendors.xml";
    //const std::string vendorlink = "http://127.0.0.1:3000/xlights_vendors.xml";

    std::string filename;
    wxXmlDocument* vd = GetXMLFromURL(wxURI(vendorlink), filename);
    if (vd == nullptr || !vd->IsOk()) {
        vd = GetXMLFromURL(wxURI(vendorlinkbackup), filename);
    }
    if (vd != nullptr && vd->IsOk()) {
        wxXmlNode* root = vd->GetRoot();

        for (auto v = root->GetChildren(); v != nullptr; v = v->GetNext()) {
            if (v->GetName().Lower() == "musicvendor") {
                int max = -1;
                std::string url = "";

                for (auto link = v->GetChildren(); link != nullptr; link = link->GetNext()) {
                    if (link->GetName().Lower() == "link") {
                        url = link->GetNodeContent().ToStdString();
                    } else if (link->GetName().Lower() == "maxitems") {
                        max = wxAtoi(link->GetNodeContent());
                    }
                }

                if (url != "") {
                    std::string vfilename;
                    wxXmlDocument* d = GetXMLFromURL(wxURI(url), vfilename);
                    if (d != nullptr) {
                        MSLVendor* mv = new MSLVendor(d, max, &VendorMusicDialog::GetCache());
                        _vendors.push_back(mv);
                        delete d;
                    }
                }
            }
        }
    }
    if (vd != nullptr) {
        delete vd;
    }

    bool found = false;

    TreeCtrl_Navigator->DeleteAllItems();
    wxTreeItemId root = TreeCtrl_Navigator->AddRoot("Vendors");
    for (const auto& it : _vendors) {
        wxTreeItemId v = TreeCtrl_Navigator->AppendItem(root, it->_name, -1, -1, new MSLVendorTreeItemData(it));
        if (hash == "") {
            AddHierachy(v, it, it->_categories);
        } else {
            if (AddSequenceLyric(v, it, hash)) {
                found = true;
            }
        }
        TreeCtrl_Navigator->Expand(v);
    }

    if (hash != "") {
        if (!found) {
            return false;
        }
    }

    if (_vendors.size() == 0) {
        DisplayError("Unable to retrieve any vendor information", this);
        return false;
    }

    return true;
}

void VendorMusicDialog::AddHierachy(wxTreeItemId id, MSLVendor* vendor, std::list<MSLVendorCategory*> categories)
{
    for (const auto& it : categories) {
        wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(id, it->_name, -1, -1, new MSLCategoryTreeItemData(it));
        AddHierachy(tid, vendor, it->_categories);
        TreeCtrl_Navigator->Expand(tid);
        AddSequenceLyricInCategory(tid, vendor, it->_id);
    }
}

bool VendorMusicDialog::AddSequenceLyric(wxTreeItemId v, MSLVendor* vendor, std::string hash)
{
    auto msl = vendor->GetSequenceLyrics(hash);

    if (msl.size() == 0) return false;

    for (const auto& it : msl)
    {
        std::string label = it->GetType();
        if (hash == "")
        {
            label = it->_title;
            if (label != "")
            {
                label += " - ";
            }
            if (it->_artist != "")
            {
                label += it->_artist;
                label += " - ";
            }
            label += it->GetType();
        }
        TreeCtrl_Navigator->AppendItem(v, label, -1, -1, new MSLSequenceLyricTreeItemData(it));
    }

    return true;
}

bool VendorMusicDialog::AddSequenceLyricInCategory(wxTreeItemId v, MSLVendor* vendor, std::string category)
{
    auto msl = vendor->GetSequenceLyricsForCategory(category);

    if (msl.size() == 0) return false;

    for (const auto& it : msl)
    {
        std::string label = it->_title;
        if (label != "")
        {
            label += " - ";
        }
        if (it->_artist != "")
        {
            label += it->_artist;
            label += " - ";
        }
        label += it->GetType();
        TreeCtrl_Navigator->AppendItem(v, label, -1, -1, new MSLSequenceLyricTreeItemData(it));
    }

    return true;
}

VendorMusicDialog::~VendorMusicDialog()
{
    //(*Destroy(VendorMusicDialog)
    //*)

    GetCache().Save();

    for (const auto& it : _vendors) {
        delete it;
    }
}

void VendorMusicDialog::OnHyperlinkCtrl_ModelWebLinkClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_WebLink->GetURL());
}

void VendorMusicDialog::OnButton_InsertModelClick(wxCommandEvent& event)
{
    if (TreeCtrl_Navigator->GetSelection().IsOk()) {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr && ((MSLVendorBaseTreeItemData*)tid)->GetType() == "SequenceLyric") {
            auto doc = ((MSLSequenceLyricTreeItemData*)tid)->GetSequenceLyric();
            auto filename = doc->Download(xLightsFrame::CurrentDir);
            if (filename != "") {
                if (doc->_title != "") {
                    auto newfile = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + doc->_title + "." + doc->GetExt();
                    if (wxRenameFile(filename, newfile, false)) {
                        filename = newfile;
                    }
                }
                wxMessageBox("File downloaded: " + filename);
            } else {
                wxMessageBox("File failed to download.");
            }
        }
    }
    // Dont exit so user can download multiple items
    //EndDialog(wxID_OK);
}

void VendorMusicDialog::OnNotebookPanelsPageChanged(wxNotebookEvent& event)
{
}

void VendorMusicDialog::OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event)
{
    ValidateWindow();
}

void VendorMusicDialog::OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event)
{
    static bool busy = false;

    if (busy) {
        return;
    }

    // Because this code triggers a web download this function can be re-entered and this is not good
    busy = true;

    wxTreeItemId startid = TreeCtrl_Navigator->GetSelection();

    SetCursor(wxCURSOR_WAIT);

    if (TreeCtrl_Navigator->GetSelection().IsOk()) {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(startid);

        if (tid != nullptr) {
            std::string type = ((MSLVendorBaseTreeItemData*)tid)->GetType();

            if (type == "Vendor") {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->GetPage(1)->Hide();
                NotebookPanels->SetSelection(0);
                PopulateVendorPanel(((MSLVendorTreeItemData*)tid)->GetVendor());
                TreeCtrl_Navigator->SetFocus();
            } else if (type == "SequenceLyric") {
                NotebookPanels->GetPage(0)->Hide();
                NotebookPanels->GetPage(1)->Show();
                NotebookPanels->SetSelection(1);
                TreeCtrl_Navigator->SetFocus();
                PopulateSequenceLyricPanel(((MSLSequenceLyricTreeItemData*)tid)->GetSequenceLyric());
            } else {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->GetPage(1)->Hide();
                NotebookPanels->SetSelection(0);
                PopulateVendorPanel(((MSLCategoryTreeItemData*)tid)->GetCategory()->GetVendor());
                TreeCtrl_Navigator->SetFocus();
            }
        } else {
            NotebookPanels->GetPage(0)->Hide();
            NotebookPanels->GetPage(1)->Hide();
        }
    } else {
        NotebookPanels->GetPage(0)->Hide();
        NotebookPanels->GetPage(1)->Hide();
    }

    ValidateWindow();

    SetCursor(wxCURSOR_DEFAULT);

    busy = false;

    if (startid != TreeCtrl_Navigator->GetSelection()) {
        // selection changed while we were processing so lets try again
        wxPostEvent(this, event);
    }
}

void VendorMusicDialog::OnHyperlinkCtrl_WebsiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_Website->GetURL());
}

void VendorMusicDialog::OnHyperlinkCtrl_FacebookClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_Facebook->GetURL());
}

void VendorMusicDialog::ValidateWindow()
{
    wxTreeItemId current = TreeCtrl_Navigator->GetSelection();
    if (TextCtrl_Search->GetValue().Trim(true).Trim(false) == "" || !current.IsOk()) {
        Button_Search->Disable();
    } else {
        Button_Search->Enable();
    }

    if (TreeCtrl_Navigator->GetSelection().IsOk()) {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr) {
            std::string type = ((MSLVendorBaseTreeItemData*)tid)->GetType();

            if (type == "SequenceLyric" && ((MSLSequenceLyricTreeItemData*)tid)->GetSequenceLyric()->_download.BuildURI() != "") {
                Button_Download->Enable();
            } else {
                Button_Download->Disable();
            }
        }
    } else {
        Button_Download->Disable();
    }
}

void VendorMusicDialog::PopulateVendorPanel(MSLVendor* vendor)
{
    if (vendor == nullptr) {
        NotebookPanels->GetPage(0)->Hide();
        return;
    }

    if (vendor->_logoFile.Exists()) {
        _vendorImage.LoadFile(vendor->_logoFile.GetFullPath());
        if (_vendorImage.IsOk()) {
            StaticBitmap_VendorImage->Show();
            LoadImage(StaticBitmap_VendorImage, &_vendorImage);
        } else {
            StaticBitmap_VendorImage->Hide();
        }
    } else {
        StaticBitmap_VendorImage->Hide();
    }

    TextCtrl_VendorDetails->SetValue(vendor->GetDescription());

    if (vendor->_facebook.GetPath() != "") {
        StaticText6->Show();
        HyperlinkCtrl_Facebook->Show();
        HyperlinkCtrl_Facebook->SetURL(vendor->_facebook.BuildURI());
        HyperlinkCtrl_Facebook->SetLabel(vendor->_facebook.BuildURI());
    } else {
        StaticText6->Hide();
        HyperlinkCtrl_Facebook->Hide();
    }
    if (vendor->_website.GetPath() != "") {
        StaticText2->Show();
        HyperlinkCtrl_Website->Show();
        HyperlinkCtrl_Website->SetURL(vendor->_website.BuildURI());
        HyperlinkCtrl_Website->SetLabel(vendor->_website.BuildURI());
    } else {
        StaticText2->Hide();
        HyperlinkCtrl_Website->Hide();
    }
    PanelVendor->Layout();
}

void VendorMusicDialog::LoadImage(wxStaticBitmap* sb, wxImage* image) const
{
    if (image->GetWidth() == 0 || image->GetHeight() == 0)
        return;

    wxImage disp = image->Copy();

    float x = (float)sb->GetSize().GetWidth() / (float)disp.GetWidth();
    float y = (float)sb->GetSize().GetHeight() / (float)disp.GetHeight();
    float scale = std::min(x, y);

    sb->SetBitmap(disp.Rescale((float)disp.GetWidth() * scale, (float)disp.GetHeight() * scale));
}

void VendorMusicDialog::PopulateSequenceLyricPanel(MSLSequenceLyric* msl)
{
    if (msl == nullptr) {
        StaticText1->Hide();
        StaticText3->Hide();
        StaticText5->Hide();
        TextCtrl_ModelDetails->Hide();
        HyperlinkCtrl_WebLink->Hide();
        HyperlinkCtrl_MusicLink->Hide();
        HyperlinkCtrl_VideoLink->Hide();
        NotebookPanels->GetPage(1)->Hide();
        return;
    }

    TextCtrl_ModelDetails->Show();
    TextCtrl_ModelDetails->SetValue(msl->GetDescription());

    if (msl->_webpage.GetPath() != "") {
        StaticText5->Show();
        HyperlinkCtrl_WebLink->Show();
        HyperlinkCtrl_WebLink->SetURL(msl->_webpage.BuildURI());
        HyperlinkCtrl_WebLink->SetLabel("View " + msl->GetType() + " at " + msl->_webpage.GetServer());
    } else {
        StaticText5->Hide();
        HyperlinkCtrl_WebLink->Hide();
    }

    if (msl->_video.GetPath() != "") {
        StaticText1->Show();
        HyperlinkCtrl_VideoLink->Show();
        HyperlinkCtrl_VideoLink->SetURL(msl->_video.BuildURI());
        HyperlinkCtrl_VideoLink->SetLabel("View video");
    } else {
        StaticText1->Hide();
        HyperlinkCtrl_VideoLink->Hide();
    }

    if (msl->_music.GetPath() != "") {
        StaticText3->Show();
        HyperlinkCtrl_MusicLink->Show();
        HyperlinkCtrl_MusicLink->SetURL(msl->_music.BuildURI());
        HyperlinkCtrl_MusicLink->SetLabel("Purchase Music");
    } else {
        StaticText3->Hide();
        HyperlinkCtrl_MusicLink->Hide();
    }

    if (msl->_download.GetPath() != "") {
        Button_Download->Show();
    } else {
        Button_Download->Hide();
    }

    Panel_Item->Layout();
}

void VendorMusicDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void VendorMusicDialog::OnResize(wxSizeEvent& event)
{
    wxDialog::OnSize(event);

    if (NotebookPanels->GetSelection() == 0) {
        if (StaticBitmap_VendorImage->IsShown() && _vendorImage.IsOk()) {
            if (_vendorImage.GetSize() != StaticBitmap_VendorImage->GetSize()) {
                LoadImage(StaticBitmap_VendorImage, &_vendorImage);
            }
        }
    }

    PanelVendor->Refresh();
}

void VendorMusicDialog::OnHyperlinkCtrl_MusicLinkClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_MusicLink->GetURL());
}

void VendorMusicDialog::OnHyperlinkCtrl_VideoLinkClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_VideoLink->GetURL());
}

void VendorMusicDialog::OnInit(wxInitDialogEvent& event)
{
}

void VendorMusicDialog::OnTextCtrl_SearchText(wxCommandEvent& event)
{
    ValidateWindow();
}

void VendorMusicDialog::OnButton_SearchClick(wxCommandEvent& event)
{
    wxString searchFor = TextCtrl_Search->GetValue().Lower();

    // cant search if tree is empty
    if (TreeCtrl_Navigator->GetChildrenCount(TreeCtrl_Navigator->GetRootItem()) == 0) {
        wxBell();
        return;
    }

    wxTreeItemId current = TreeCtrl_Navigator->GetSelection();
    wxTreeItemId start = current;
    if (current.IsOk()) {
        do {
            // if this node has children got to the first child
            if (TreeCtrl_Navigator->GetChildrenCount(current, false) > 0) {
                wxTreeItemIdValue cookie;
                current = TreeCtrl_Navigator->GetFirstChild(current, cookie);
            } else {
                // no child ... so go to the sibling
                auto sibling = TreeCtrl_Navigator->GetNextSibling(current);
                if (sibling.IsOk()) {
                    current = sibling;
                } else {
                    // no sibling so we need to move up until a sibling exists and then get the next one.
                    for (;;) {
                        auto parent = TreeCtrl_Navigator->GetItemParent(current);
                        if (parent == TreeCtrl_Navigator->GetRootItem()) {
                            current = parent;
                            break;
                        } else if (parent.IsOk()) {
                            sibling = TreeCtrl_Navigator->GetNextSibling(parent);
                            if (sibling.IsOk()) {
                                current = sibling;
                                break;
                            } else {
                                current = parent;
                                if (current == TreeCtrl_Navigator->GetRootItem()) {
                                    break;
                                }
                            }
                        } else {
                            wxBell();
                            return;
                        }
                    }
                }
            }

            if (current != TreeCtrl_Navigator->GetRootItem() && TreeCtrl_Navigator->GetItemText(current).Lower().Contains(TextCtrl_Search->GetValue().Lower())) {
                TreeCtrl_Navigator->SelectItem(current);
                TreeCtrl_Navigator->EnsureVisible(current);
                if (current == start) {
                    wxBell();
                }
                return;
            }

        } while (current != start);
        wxBell();
    }
}
