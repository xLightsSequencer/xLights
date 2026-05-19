/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VendorModelDialog.h"

//(*InternalHeaders(VendorModelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <algorithm>
#include <log.h>
#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include <wx/progdlg.h>
#include "settings/XLightsConfigAdapter.h"
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/regex.h>

#include "CachedFileDownloader.h"
#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include "utils/ExternalHooks.h"
#include "import_export/VendorCatalog.h"

// R-VendorCatalog (2026-05-13): the data classes + XML parser
// for the vendor catalog moved to `src-core/import_export/`
// (single-source with iPad's vendor-browser sheet). Local M*
// aliases preserve the rest of this dialog's call sites.
using MVendor = vendor_catalog::Vendor;
using MModel = vendor_catalog::Model;
using MModelWiring = vendor_catalog::ModelWiring;
using MVendorCategory = vendor_catalog::Category;

// Replaces the legacy `MModel::GetColour()` / `MModelWiring::GetColour()`
// methods which lived on the data classes when they used `wxColour`.
// Orange = no downloadable xmodel; cyan-or-blue = a wiring exists.
static wxColour TreeItemColourForModel(const MModel* m) {
    return m->HasDownloadableXmodel() ? CyanOrBlue() : wxColour(255, 128, 0);
}

// Extract the host portion ("example.com") from a URL string.
// Replaces `wxURI::GetServer()` for the link-formatting paths.
static std::string ExtractURLServer(const std::string& url) {
    const auto schemeEnd = url.find("://");
    const size_t hostStart = (schemeEnd == std::string::npos) ? 0 : schemeEnd + 3;
    const auto hostEnd = url.find('/', hostStart);
    return (hostEnd == std::string::npos)
        ? url.substr(hostStart)
        : url.substr(hostStart, hostEnd - hostStart);
}

class VendorBaseTreeItemData : public wxTreeItemData
{
public:
    VendorBaseTreeItemData(std::string type) : _type(type) { }

    std::string GetType() const { return _type; }

private:
    std::string _type;
};

class MVendorTreeItemData : public VendorBaseTreeItemData
{
public:
    MVendorTreeItemData(MVendor* vendor) : VendorBaseTreeItemData("Vendor"), _vendor(vendor) { }

    MVendor* GetVendor() const { return _vendor; }

private:
    MVendor * _vendor;
};

class MCategoryTreeItemData : public VendorBaseTreeItemData
{
public:
    MCategoryTreeItemData(MVendorCategory* category) : VendorBaseTreeItemData("Category"), _category(category) { }

    MVendorCategory* GetCategory() const { return _category; }

private:
    MVendorCategory * _category;
};

class MModelTreeItemData : public VendorBaseTreeItemData
{
public:
    MModelTreeItemData(MModel* model) : VendorBaseTreeItemData("Model"), _model(model) { }

    MModel* GetModel() const { return _model; }

private:
    MModel * _model;
};

class MWiringTreeItemData : public VendorBaseTreeItemData
{
public:
    MWiringTreeItemData(MModelWiring* wiring) : VendorBaseTreeItemData("Wiring"), _wiring(wiring) { }

    MModelWiring* GetWiring() const { return _wiring; }

private:
    MModelWiring* _wiring;
};

//(*IdInit(VendorModelDialog)
const wxWindowID VendorModelDialog::ID_TREECTRL1 = wxNewId();
const wxWindowID VendorModelDialog::ID_TEXTCTRL3 = wxNewId();
const wxWindowID VendorModelDialog::ID_PANEL3 = wxNewId();
const wxWindowID VendorModelDialog::ID_CHECKBOX1 = wxNewId();
const wxWindowID VendorModelDialog::ID_STATICBITMAP1 = wxNewId();
const wxWindowID VendorModelDialog::ID_TEXTCTRL1 = wxNewId();
const wxWindowID VendorModelDialog::ID_STATICTEXT8 = wxNewId();
const wxWindowID VendorModelDialog::ID_HYPERLINKCTRL4 = wxNewId();
const wxWindowID VendorModelDialog::ID_STATICTEXT4 = wxNewId();
const wxWindowID VendorModelDialog::ID_HYPERLINKCTRL2 = wxNewId();
const wxWindowID VendorModelDialog::ID_PANEL2 = wxNewId();
const wxWindowID VendorModelDialog::ID_BUTTON2 = wxNewId();
const wxWindowID VendorModelDialog::ID_STATICBITMAP2 = wxNewId();
const wxWindowID VendorModelDialog::ID_ANIMATIONCTRL1 = wxNewId();
const wxWindowID VendorModelDialog::ID_BUTTON3 = wxNewId();
const wxWindowID VendorModelDialog::ID_PANEL5 = wxNewId();
const wxWindowID VendorModelDialog::ID_TEXTCTRL2 = wxNewId();
const wxWindowID VendorModelDialog::ID_STATICTEXT7 = wxNewId();
const wxWindowID VendorModelDialog::ID_HYPERLINKCTRL3 = wxNewId();
const wxWindowID VendorModelDialog::ID_BUTTON1 = wxNewId();
const wxWindowID VendorModelDialog::ID_PANEL4 = wxNewId();
const wxWindowID VendorModelDialog::ID_NOTEBOOK1 = wxNewId();
const wxWindowID VendorModelDialog::ID_PANEL1 = wxNewId();
const wxWindowID VendorModelDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VendorModelDialog,wxDialog)
	//(*EventTable(VendorModelDialog)
	//*)
END_EVENT_TABLE()

VendorModelDialog::VendorModelDialog(wxWindow* parent, const std::string& showFolder, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _showFolder = showFolder;

    //(*Initialize(VendorModelDialog)
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;


    Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
    SetClientSize(wxSize(800,600));
    Move(wxDefaultPosition);
    SetMinSize(wxSize(800,400));
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_LIVE_UPDATE, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetMinimumPaneSize(10);
    SplitterWindow1->SetSashGravity(0.5);
    Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(0);
    TreeCtrl_Navigator = new wxTreeCtrl(Panel3, ID_TREECTRL1, wxDefaultPosition, wxSize(200,-1), wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT|wxTR_ROW_LINES|wxTR_MULTIPLE|wxTR_DEFAULT_STYLE|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_TREECTRL1"));
    FlexGridSizer2->Add(TreeCtrl_Navigator, 1, wxALL|wxEXPAND, 5);
    TextCtrl_Search = new wxSearchCtrl(Panel3, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    TextCtrl_Search->SetDescriptiveText("Search models...");
    TextCtrl_Search->ShowCancelButton(true);
    FlexGridSizer2->Add(TextCtrl_Search, 0, wxALL|wxEXPAND, 5);
    Panel3->SetSizer(FlexGridSizer2);
    Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(0);
    NotebookPanels = new wxNotebook(Panel1, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    PanelVendor = new wxPanel(NotebookPanels, ID_PANEL2, wxPoint(43,60), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer4->AddGrowableCol(0);
    FlexGridSizer4->AddGrowableRow(1);
    CheckBox_DontDownload = new wxCheckBox(PanelVendor, ID_CHECKBOX1, _("Don\'t download this vendors list of models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_DontDownload->SetValue(false);
    FlexGridSizer4->Add(CheckBox_DontDownload, 1, wxALL|wxEXPAND, 5);
    StaticBitmap_VendorImage = new wxStaticBitmap(PanelVendor, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(256,128), 0, _T("ID_STATICBITMAP1"));
    StaticBitmap_VendorImage->SetMinSize(wxSize(256,128));
    FlexGridSizer4->Add(StaticBitmap_VendorImage, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_VendorDetails = new wxTextCtrl(PanelVendor, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer4->Add(TextCtrl_VendorDetails, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(1);
    FlexGridSizer5->AddGrowableRow(0);
    StaticText6 = new wxStaticText(PanelVendor, ID_STATICTEXT8, _("Facebook:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer5->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_Facebook = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL4, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxBORDER_NONE, _T("ID_HYPERLINKCTRL4"));
    FlexGridSizer5->Add(HyperlinkCtrl_Facebook, 1, wxALL|wxEXPAND, 5);
    StaticText2 = new wxStaticText(PanelVendor, ID_STATICTEXT4, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer5->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_Website = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL2, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxBORDER_NONE, _T("ID_HYPERLINKCTRL2"));
    FlexGridSizer5->Add(HyperlinkCtrl_Website, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
    PanelVendor->SetSizer(FlexGridSizer4);
    Panel_Item = new wxPanel(NotebookPanels, ID_PANEL4, wxPoint(41,9), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    FlexGridSizer6->AddGrowableRow(1);
    ItemImagePanel = new wxPanel(Panel_Item, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer7->AddGrowableCol(1);
    FlexGridSizer7->AddGrowableRow(0);
    Button_Prior = new wxButton(ItemImagePanel, ID_BUTTON2, _T("<"), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer7->Add(Button_Prior, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticBitmap_ModelImage = new wxStaticBitmap(ItemImagePanel, ID_STATICBITMAP2, wxNullBitmap, wxDefaultPosition, wxSize(256,256), 0, _T("ID_STATICBITMAP2"));
    StaticBitmap_ModelImage->SetMinSize(wxSize(256,256));
    FlexGridSizer10->Add(StaticBitmap_ModelImage, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
        wxAnimation anim_1(wxEmptyString);
    AnimationCtrl1 = new wxAnimationCtrl(ItemImagePanel, ID_ANIMATIONCTRL1, anim_1, wxDefaultPosition, wxSize(256,256), wxAC_DEFAULT_STYLE, _T("ID_ANIMATIONCTRL1"));
    AnimationCtrl1->SetMinSize(wxSize(256,256));
    FlexGridSizer10->Add(AnimationCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Next = new wxButton(ItemImagePanel, ID_BUTTON3, _T(">"), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(Button_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ItemImagePanel->SetSizer(FlexGridSizer7);
    FlexGridSizer6->Add(ItemImagePanel, 1, wxALL|wxEXPAND, 0);
    TextCtrl_ModelDetails = new wxTextCtrl(Panel_Item, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer6->Add(TextCtrl_ModelDetails, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText5 = new wxStaticText(Panel_Item, ID_STATICTEXT7, _("Web Link:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer8->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_ModelWebLink = new wxHyperlinkCtrl(Panel_Item, ID_HYPERLINKCTRL3, _("https://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxBORDER_NONE, _T("ID_HYPERLINKCTRL3"));
    FlexGridSizer8->Add(HyperlinkCtrl_ModelWebLink, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
    Button_InsertModel = new wxButton(Panel_Item, ID_BUTTON1, _("Insert Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer6->Add(Button_InsertModel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel_Item->SetSizer(FlexGridSizer6);
    NotebookPanels->AddPage(PanelVendor, _("Vendor"), false);
    NotebookPanels->AddPage(Panel_Item, _("Item"), false);
    FlexGridSizer3->Add(NotebookPanels, 1, wxALL|wxEXPAND, 5);
    Panel1->SetSizer(FlexGridSizer3);
    SplitterWindow1->SplitVertically(Panel3, Panel1);
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_TREECTRL1, wxEVT_COMMAND_TREE_ITEM_ACTIVATED, (wxObjectEventFunction)&VendorModelDialog::OnTreeCtrl_NavigatorItemActivated);
    Connect(ID_TREECTRL1, wxEVT_COMMAND_TREE_SEL_CHANGED, (wxObjectEventFunction)&VendorModelDialog::OnTreeCtrl_NavigatorSelectionChanged);
    TextCtrl_Search->Bind(wxEVT_TEXT, &VendorModelDialog::OnTextCtrl_SearchText, this);
    TextCtrl_Search->Bind(wxEVT_SEARCHCTRL_SEARCH_BTN, &VendorModelDialog::OnButton_SearchClick, this);
    TextCtrl_Search->Bind(wxEVT_SEARCHCTRL_CANCEL_BTN, &VendorModelDialog::OnSearchCancelClick, this);
    Connect(ID_CHECKBOX1, wxEVT_COMMAND_CHECKBOX_CLICKED, (wxObjectEventFunction)&VendorModelDialog::OnCheckBox_DontDownloadClick);
    Connect(ID_HYPERLINKCTRL4, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorModelDialog::OnHyperlinkCtrl_FacebookClick);
    Connect(ID_HYPERLINKCTRL2, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorModelDialog::OnHyperlinkCtrl_WebsiteClick);
    Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VendorModelDialog::OnButton_PriorClick);
    Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VendorModelDialog::OnButton_NextClick);
    Connect(ID_HYPERLINKCTRL3, wxEVT_COMMAND_HYPERLINK, (wxObjectEventFunction)&VendorModelDialog::OnHyperlinkCtrl_ModelWebLinkClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&VendorModelDialog::OnButton_InsertModelClick);
    Connect(ID_NOTEBOOK1, wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED, (wxObjectEventFunction)&VendorModelDialog::OnNotebookPanelsPageChanged);
    Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&VendorModelDialog::OnClose);
    Connect(wxEVT_SIZE, (wxObjectEventFunction)&VendorModelDialog::OnResize);
    //*)

    SetSize(800, 600);

    PopulateModelPanel((MModel*)nullptr);
    PopulateVendorPanel(nullptr);

    HyperlinkCtrl_Facebook->SetNormalColour(CyanOrBlue());
    HyperlinkCtrl_Website->SetNormalColour(CyanOrBlue());
    HyperlinkCtrl_ModelWebLink->SetNormalColour(CyanOrBlue());
    ValidateWindow();
}

bool VendorModelDialog::DlgInit(wxProgressDialog* prog, int low, int high)
{
    if (LoadTree(prog, low, high)) {
        ValidateWindow();
        return true;
    }

    return false;
}
bool VendorModelDialog::FindModelFile(const std::string &vendor, const std::string &model) {
    for (auto &v : _vendors) {
        if (v->_name == vendor) {
            for (auto &m: v->_models) {
                if (m->_name == model && !m->_wiring.empty()) {
                    (void)DownloadModel(m->_wiring.front());
                    if (_modelFile != "") {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

pugi::xml_document* VendorModelDialog::GetXMLFromURL(wxURI url, std::string& filename, wxProgressDialog* prog, int low, int high, bool keepProgress) const
{
    filename = "";
    std::function<bool(int)> progressFn;
    if (prog) {
        progressFn = [prog](int value) -> bool { return prog->Update(value); };
    }
    wxFileName fn = wxFileName(VendorModelDialog::GetCache().GetFile(url.BuildURI().ToStdString(), CACHEFOR::CACHETIME_SESSION, "", progressFn, low, high, keepProgress));
    if (FileExists(fn)) {
        filename = fn.GetFullPath();
        auto doc = new pugi::xml_document();
        if (!doc->load_file(filename.c_str())) {
            delete doc;
            return nullptr;
        }
        return doc;
    }

    return nullptr;
}

bool VendorModelDialog::LoadTree(wxProgressDialog* prog, int low, int high)
{
    const std::string vendorlink = "https://raw.githubusercontent.com/xLightsSequencer/xLights/master/download/xlights_vendors.xml";
    const std::string vendorlinkbackup = "https://nutcracker123.com/xlights/vendors/xlights_vendors.xml";
    //const std::string vendorlink = "http://localhost/xlights_vendors.xml";

    std::string filename;
    if (prog != nullptr)
        prog->Update(low, "Downloading vendor list");
    pugi::xml_document* vd = GetXMLFromURL(wxURI(vendorlink), filename, prog, low, high, true);
    if (prog != nullptr)
        prog->Update(high, "Parsing vendor list");

    if (vd == nullptr || !vd->document_element()) {
        delete vd;
        vd = GetXMLFromURL(wxURI(vendorlinkbackup), filename, prog, low, high, true);
    }

    if (vd != nullptr && vd->document_element()) {
        pugi::xml_node root = vd->document_element();

        for (pugi::xml_node v = root.first_child(); v; v = v.next_sibling())
        {
            if (::Lower(v.name()) == "vendor")
            {
                int maxModels = -1;
                std::string url = "";
                std::string name = "";

                for (pugi::xml_node link = v.first_child(); link; link = link.next_sibling())
                {
                    if (::Lower(link.name()) =="link")
                    {
                        url = link.text().get();
                    }
                    else if (::Lower(link.name()) =="maxmodels")
                    {
                        maxModels = link.text().as_int();
                    }
                    else if (::Lower(link.name()) =="name")
                    {
                        name = link.text().get();
                    }
                }

                if (name != "" && IsVendorSuppressed(name))
                {
                    MVendor* mv = new MVendor(name);
                    _vendors.push_back(mv);

                    spdlog::debug("Vendor {} not downloaded as suppressed.", name);
                }
                else
                {
                    if (url != "") {
                        std::string vfilename;
                        if (prog != nullptr)
                            prog->Update(low, "Downloading " + name + " data.");
                        pugi::xml_document* d = GetXMLFromURL(wxURI(url), vfilename, prog, low, high, true);
                        if (d != nullptr && d->document_element()) {
                            if (prog != nullptr)
                                prog->Update(high, "Parsing " + name + " data.");
                            MVendor* mv = new MVendor(*d, maxModels);
                            _vendors.push_back(mv);
                            delete d;
                            spdlog::debug("Vendor {} downloaded.", name);
                        } else {
                            delete d;
                            spdlog::debug("Vendor {} failed to download or validate.", name);
                        }
                    }
                    else
                    {
                        spdlog::debug("Vendor {} has no url for its models.", name);
                    }
                }
            }
        }
    }
    if (prog != nullptr) prog->Update(100);
    if (vd != nullptr)
    {
        delete vd;
    }

    TreeCtrl_Navigator->Freeze();

    TreeCtrl_Navigator->DeleteAllItems();
    wxTreeItemId root = TreeCtrl_Navigator->AddRoot("Vendors");
    wxTreeItemId first = root;
    for (const auto& it : _vendors)
    {
        wxTreeItemId v = TreeCtrl_Navigator->AppendItem(root, it->_name, -1, -1, new MVendorTreeItemData(it));
        if (first == root)
        {
            first = v;
        }
        if (!IsVendorSuppressed(it->_name))
        {
            AddHierachy(v, it, it->_categories);
        }
    }

    if (first.IsOk() && first != root)
    {
        TreeCtrl_Navigator->EnsureVisible(first);
    }

    wxTreeItemIdValue cookie;
    for (auto l1 = TreeCtrl_Navigator->GetFirstChild(root, cookie); l1.IsOk(); l1 = TreeCtrl_Navigator->GetNextChild(root, cookie))
    {
        UNUSED(DeleteEmptyCategories(l1));
    }

    TreeCtrl_Navigator->Thaw();

    if (_vendors.size() == 0)
    {
        DisplayError("Unable to retrieve any vendor information", this);
        return false;
    }

    return true;
}

bool VendorModelDialog::DeleteEmptyCategories(wxTreeItemId& parent)
{
    VendorBaseTreeItemData* tid = (VendorBaseTreeItemData*)TreeCtrl_Navigator->GetItemData(parent);
    if (tid->GetType() == "Category" && TreeCtrl_Navigator->GetChildrenCount(parent) == 0)
    {
        TreeCtrl_Navigator->Delete(parent);
        return true;
    }
    else if (tid->GetType() == "Category" || tid->GetType() == "Vendor")
    {
        wxTreeItemIdValue cookie;
        for (auto l1 = TreeCtrl_Navigator->GetFirstChild(parent, cookie);
            l1.IsOk();
            )
        {
            auto next = TreeCtrl_Navigator->GetNextChild(parent, cookie);
            UNUSED(DeleteEmptyCategories(l1));
            l1 = next;
        }
    }
    return false;
}

void VendorModelDialog::AddHierachy(wxTreeItemId id, MVendor* vendor, std::list<MVendorCategory*> categories)
{
    for (const auto& it : categories)
    {
        wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(id, it->_name, -1, -1, new MCategoryTreeItemData(it));
        AddHierachy(tid, vendor, it->_categories);
        TreeCtrl_Navigator->Expand(tid);
        AddModels(tid, vendor, it->_id);
    }
}

void VendorModelDialog::AddModels(wxTreeItemId v, MVendor* vendor, std::string categoryId)
{
    auto models = vendor->GetModels(categoryId);

    for (const auto& it : models)
    {
        if (it->_wiring.size() > 1)
        {
            wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(v, it->_name, -1, -1, new MModelTreeItemData(it));
            for (const auto& it2 : it->_wiring)
            {
                wxTreeItemId id = TreeCtrl_Navigator->AppendItem(tid, it2->_name, -1, -1, new MWiringTreeItemData(it2));
                TreeCtrl_Navigator->SetItemTextColour(id, TreeItemColourForModel(it));
            }
        }
        else
        {
            if (it->_wiring.size() == 0)
            {
                wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(v, it->_name, -1, -1, new MModelTreeItemData(it));
                TreeCtrl_Navigator->SetItemTextColour(tid, TreeItemColourForModel(it));
            }
            else
            {
                wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(v, it->_name, -1, -1, new MWiringTreeItemData(it->_wiring.front()));
                TreeCtrl_Navigator->SetItemTextColour(tid, TreeItemColourForModel(it));
            }
        }
    }
}

VendorModelDialog::~VendorModelDialog()
{
	//(*Destroy(VendorModelDialog)
	//*)

    GetCache().Save();

    for (const auto& it : _vendors) {
        delete it;
    }
}

void VendorModelDialog::OnHyperlinkCtrl_ModelWebLinkClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_ModelWebLink->GetURL());
}

void VendorModelDialog::OnButton_PriorClick(wxCommandEvent& event)
{
    if (GetFocusedItem().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(GetFocusedItem());

        if (tid != nullptr)
        {
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Model")
            {
                _currImage--;
                LoadModelImage(((MModelTreeItemData*)tid)->GetModel()->_imageFiles, _currImage);
            }
            else if (type == "Wiring")
            {
                _currImage--;
                LoadModelImage(((MWiringTreeItemData*)tid)->GetWiring()->_imageFiles, _currImage);
            }
            ValidateWindow();
        }
    }
}

void VendorModelDialog::OnButton_NextClick(wxCommandEvent& event)
{
    if (GetFocusedItem().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(GetFocusedItem());

        if (tid != nullptr)
        {
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Model")
            {
                _currImage++;
                LoadModelImage(((MModelTreeItemData*)tid)->GetModel()->_imageFiles, _currImage);
            }
            else if (type == "Wiring")
            {
                _currImage++;
                LoadModelImage(((MWiringTreeItemData*)tid)->GetWiring()->_imageFiles, _currImage);
            }
            ValidateWindow();
        }
    }
}

bool VendorModelDialog::DownloadModel(MModelWiring* wiring)
{

    wiring->DownloadXModel();

    // If the download was cancelled or otherwise failed, the xmodel file won't exist on disk
    if (!FileExists(wiring->_xmodelFile)) {
        return false;
    }

    const wxFileName xmodelPath(wiring->_xmodelFile);
    if (xmodelPath.GetExt().Lower() == "zip") {
        // we need to open the zip ... place the files in the "modeldownload" folder in the show folder
        spdlog::debug("    opening zipped model " + _modelFile);

        auto dir = _showFolder + GetPathSeparator() + "modeldownload";
        if (!wxDir::Exists(dir)) {
            spdlog::debug("Creating modeldownload directory " + dir);
            wxMkdir(dir);
        }

        // files should be .jpg/png/bmp/obj ... and one xmodel file
        wxFileInputStream fin(xmodelPath.GetFullPath());
        if (fin.IsOk()) {
            wxZipInputStream zin(fin);
            if (zin.IsOk()) {
                wxZipEntry* ent = zin.GetNextEntry();
                while (ent != nullptr) {

                    // create any needed subfolders
                    if (ent->GetName().Contains('\\') || ent->GetName().Contains('/')) {
                        wxArrayString aa = wxSplit(ent->GetName(), '\\');
                        wxArrayString bb = wxSplit(ent->GetName(), '/');

                        auto createdirs = [](const wxString& parent, const wxString& sub) {
                            auto d = parent + wxFileName::GetPathSeparator() + sub;
                            if (!wxDir::Exists(d)) {
                                spdlog::debug("Creating modeldownload subdirectory " + d.ToStdString());
                                wxMkdir(d);
                            }
                            return d;
                        };

                        wxString parent = dir;
                        for (size_t i = 0; i < aa.size() - 1; i++) {
                            parent = createdirs(parent, aa[i]);
                        }

                        parent = dir;
                        for (size_t i = 0; i < bb.size() - 1; i++) {
                            parent = createdirs(parent, bb[i]);
                        }
                    }

                    auto file = dir + GetPathSeparator() + ent->GetName();
                    if (wxFileName(ent->GetName()).GetExt().Lower() == "xmodel") {
                        _modelFile = file;
                        _modelWidthMM = wiring->GetWidthMM();
                        _modelHeightMM = wiring->GetHeightMM();
                        _modelDepthMM = wiring->GetDepthMM();
                    }

                    if (!FileExists(file)) {
                        spdlog::debug("        model file " + file.ToStdString() + " downloaded.");
                        wxFileOutputStream fout(file);
                        zin.Read(fout);
                    }
                    else {
                        spdlog::warn("        skipping file " + file.ToStdString() + " it already exists.");
                    }

                    ent = zin.GetNextEntry();
                }
            }
            else {
                spdlog::error("Failed to open zip file.");
                return false;
            }
        }
        else {
            spdlog::error("Failed to open zip file.");
            return false;
        }
    }
    else {
        _modelFile = wiring->_xmodelFile;
        _modelWidthMM = wiring->GetWidthMM();
        _modelHeightMM = wiring->GetHeightMM();
        _modelDepthMM = wiring->GetDepthMM();
    }
    return !_modelFile.empty();
}

wxTreeItemId VendorModelDialog::GetFocusedItem() const
{
    // Use wxTreeCtrl's native focused-item API (confirmed available and used elsewhere
    // in the codebase, e.g. SeqSettingsDialog.cpp). This returns the actual focused
    // item rather than guessing from the last selection, which matters for multi-select
    // trees where selection ordering is not guaranteed.
    return TreeCtrl_Navigator->GetFocusedItem();
}

std::vector<MModelWiring*> VendorModelDialog::GetSelectedWirings()
{
    std::vector<MModelWiring*> wirings;
    wxArrayTreeItemIds selections;
    TreeCtrl_Navigator->GetSelections(selections);

    for (size_t i = 0; i < selections.GetCount(); i++) {
        wxTreeItemId itemId = selections[i];
        if (!itemId.IsOk()) continue;

        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(itemId);
        if (tid == nullptr) continue;

        std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

        MModelWiring* wiring = nullptr;
        if (type == "Wiring") {
            wiring = ((MWiringTreeItemData*)tid)->GetWiring();
        } else if (type == "Model") {
            MModel* model = ((MModelTreeItemData*)tid)->GetModel();
            if (model->_wiring.size() == 1) {
                wiring = model->_wiring.front();
            }
        }

        // Avoid duplicates (e.g. user selects both a Model node and its child Wiring node)
        if (wiring != nullptr && std::find(wirings.begin(), wirings.end(), wiring) == wirings.end()) {
            wirings.push_back(wiring);
        }
    }

    return wirings;
}

void VendorModelDialog::DownloadSelectedModels()
{
    _downloadedModels.clear();

    auto wirings = GetSelectedWirings();
    for (auto* wiring : wirings) {
        _modelFile.clear();
        _modelWidthMM = -1;
        _modelHeightMM = -1;
        _modelDepthMM = -1;

        // If any download in the batch is cancelled or fails, abort the whole batch
        // and leave a clean state (no partial results) per the PR's intended behavior
        if (!DownloadModel(wiring)) {
            _downloadedModels.clear();
            _modelFile.clear();
            _modelWidthMM = -1;
            _modelHeightMM = -1;
            _modelDepthMM = -1;
            return;
        }

        _downloadedModels.push_back({_modelFile, _modelWidthMM, _modelHeightMM, _modelDepthMM});
    }

    // Populate legacy single-model fields from first result for backward compatibility
    if (!_downloadedModels.empty()) {
        _modelFile = _downloadedModels[0].modelFile;
        _modelWidthMM = _downloadedModels[0].widthMM;
        _modelHeightMM = _downloadedModels[0].heightMM;
        _modelDepthMM = _downloadedModels[0].depthMM;
    }
}

void VendorModelDialog::OnButton_InsertModelClick(wxCommandEvent& event)
{
    auto wirings = GetSelectedWirings();
    if (wirings.empty()) {
        return;
    }

    DownloadSelectedModels();

    if (_downloadedModels.empty()) {
        return;
    }

    EndDialog(wxID_OK);
}

void VendorModelDialog::OnNotebookPanelsPageChanged(wxNotebookEvent& event)
{
}

void VendorModelDialog::OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event)
{
    wxTreeItemId startid = event.GetItem();

    SetCursor(wxCURSOR_WAIT);

    if (startid.IsOk()) {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(startid);

        if (tid != nullptr) {
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            MModelWiring* wiring = nullptr;

            if (type == "Model") {
                MModel* model = ((MModelTreeItemData*)tid)->GetModel();
                if (model->_wiring.size() == 1) {
                    wiring = model->_wiring.front();
                }
            }
            else if (type == "Wiring") {
                wiring = ((MWiringTreeItemData*)tid)->GetWiring();
            }

            if (wiring != nullptr) {
                if (DownloadModel(wiring)) {
                    EndDialog(wxID_OK);
                }
            }
        }
    }

    SetCursor(wxCURSOR_DEFAULT);

    ValidateWindow();
}

void VendorModelDialog::OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event)
{
    wxTreeItemId startid = event.GetItem();
    if (!startid.IsOk()) {
        startid = GetFocusedItem();
    }
    // User manually selected a different item — clear the search bold/cursor.
    if (_lastSearchItem.IsOk() && startid != _lastSearchItem) {
        TreeCtrl_Navigator->SetItemBold(_lastSearchItem, false);
        _lastSearchItem = wxTreeItemId();
    }
    UpdatePanelForItem(startid);
}

void VendorModelDialog::UpdatePanelForItem(wxTreeItemId item)
{
    static bool busy = false;
    if (busy) return;
    busy = true;

    SetCursor(wxCURSOR_WAIT);

    if (item.IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(item);

        if (tid != nullptr)
        {
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Vendor")
            {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->GetPage(1)->Hide();
                NotebookPanels->SetSelection(0);
                PopulateVendorPanel(((MVendorTreeItemData*)tid)->GetVendor());
                PopulateModelPanel((MModel*)nullptr);
            }
            else if (type == "Model")
            {
                NotebookPanels->GetPage(0)->Hide();
                NotebookPanels->GetPage(1)->Show();
                NotebookPanels->SetSelection(1);
                PopulateModelPanel(((MModelTreeItemData*)tid)->GetModel());
                PopulateVendorPanel(((MModelTreeItemData*)tid)->GetModel()->_vendor);
            }
            else if (type == "Wiring")
            {
                NotebookPanels->GetPage(0)->Hide();
                NotebookPanels->GetPage(1)->Show();
                NotebookPanels->SetSelection(1);
                PopulateModelPanel(((MWiringTreeItemData*)tid)->GetWiring());
                PopulateVendorPanel(((MWiringTreeItemData*)tid)->GetWiring()->_model->_vendor);
            }
            else
            {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->GetPage(1)->Hide();
                NotebookPanels->SetSelection(0);
                PopulateVendorPanel(((MCategoryTreeItemData*)tid)->GetCategory()->_vendor);
                PopulateModelPanel((MModel*)nullptr);
            }
        }
        else
        {
            NotebookPanels->GetPage(0)->Hide();
            NotebookPanels->GetPage(1)->Hide();
        }
    }
    else
    {
        NotebookPanels->GetPage(0)->Hide();
        NotebookPanels->GetPage(1)->Hide();
    }

    ValidateWindow();
    SetCursor(wxCURSOR_DEFAULT);
    busy = false;
}

void VendorModelDialog::OnHyperlinkCtrl_WebsiteClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_Website->GetURL());
}

void VendorModelDialog::OnHyperlinkCtrl_FacebookClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_Facebook->GetURL());
}

void VendorModelDialog::ValidateWindow()
{

    // Check multi-select for Insert button state and label
    auto wirings = GetSelectedWirings();
    if (wirings.size() > 1) {
        Button_InsertModel->Enable();
        Button_InsertModel->SetLabel(wxString::Format("Insert %zu Models", wirings.size()));
    } else if (wirings.size() == 1) {
        Button_InsertModel->Enable();
        Button_InsertModel->SetLabel("Insert Model");
    } else {
        Button_InsertModel->Disable();
        Button_InsertModel->SetLabel("Insert Model");
    }

    // Recalculate layout once after label/state changes so the button resizes
    // to fit the wider "Insert N Models" text. Guard against null parents/sizers.
    if (wxWindow* parent = Button_InsertModel->GetParent()) {
        if (wxSizer* sizer = parent->GetSizer()) {
            sizer->Layout();
        }
    }

    if (GetFocusedItem().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(GetFocusedItem());

        if (tid != nullptr)
        {
            int imageCount = 0;
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Wiring")
            {
                imageCount = ((MWiringTreeItemData*)tid)->GetWiring()->_imageFiles.size();
            }

            if (type == "Model")
            {
                imageCount = ((MModelTreeItemData*)tid)->GetModel()->_imageFiles.size();
            }

            if (_currImage > 0)
            {
                Button_Prior->Enable();
            }
            else
            {
                Button_Prior->Disable();
            }

            if (_currImage < imageCount - 1)
            {
                Button_Next->Enable();
            }
            else
            {
                Button_Next->Disable();
            }
        }
    }
    else
    {
        Button_Next->Disable();
        Button_Prior->Disable();
    }
}

void VendorModelDialog::PopulateVendorPanel(MVendor* vendor)
{
    if (vendor == nullptr)
    {
        NotebookPanels->GetPage(0)->Hide();
        return;
    }

    CheckBox_DontDownload->SetValue(IsVendorSuppressed(vendor->_name));

    if (FileExists(vendor->_logoFile))
    {
        _vendorImage.LoadFile(wxString(vendor->_logoFile));
        if (_vendorImage.IsOk())
        {
            StaticBitmap_VendorImage->Show();
            LoadImage(StaticBitmap_VendorImage, &_vendorImage);
        }
        else
        {
            StaticBitmap_VendorImage->Hide();
        }
    }
    else
    {
        StaticBitmap_VendorImage->Hide();
    }

    TextCtrl_VendorDetails->SetValue(vendor->GetDescription());

    if (!vendor->_facebook.empty())
    {
        StaticText6->Show();
        HyperlinkCtrl_Facebook->Show();
        HyperlinkCtrl_Facebook->SetURL(vendor->_facebook);
        HyperlinkCtrl_Facebook->SetLabel(vendor->_facebook);
    }
    else
    {
        StaticText6->Hide();
        HyperlinkCtrl_Facebook->Hide();
    }
    if (!vendor->_website.empty())
    {
        StaticText2->Show();
        HyperlinkCtrl_Website->Show();
        HyperlinkCtrl_Website->SetURL(vendor->_website);
        HyperlinkCtrl_Website->SetLabel(vendor->_website);
    }
    else
    {
        StaticText2->Hide();
        HyperlinkCtrl_Website->Hide();
    }
    PanelVendor->Layout();
}

void VendorModelDialog::LoadImage(wxStaticBitmap* sb, wxImage* image) const
{
    if (image->GetWidth() == 0 || image->GetHeight() == 0) return;

    wxImage disp = image->Copy();

    float x = (float)sb->GetSize().GetWidth() / (float)disp.GetWidth();
    float y = (float)sb->GetSize().GetHeight() / (float)disp.GetHeight();
    float scale = std::min(x, y);

    sb->SetBitmap(disp.Rescale((float)disp.GetWidth() * scale, (float)disp.GetHeight() * scale));
}

void VendorModelDialog::LoadModelImage(const std::list<std::string>& imageFiles, int image)
{
    wxAnimation animation;

    if (AnimationCtrl1->IsPlaying()) {
        AnimationCtrl1->Stop();
    }

    auto it = imageFiles.begin();
    for (int i = 0; i < image; i++) {
        ++it;
    }

    if (FileExists(*it)) {
        const wxFileName fn(*it);
        wxString ext = fn.GetExt().Lower();
        bool isAnimation = false;
        if (ext != "jpg" && ext != "jpeg" && ext != "png" && ext != "bmp") {
            if (animation.LoadFile(fn.GetFullPath(), wxANIMATION_TYPE_GIF)) {
                isAnimation = true;
                StaticBitmap_ModelImage->Hide();
                AnimationCtrl1->Show();
                AnimationCtrl1->SetAnimation(animation);
                AnimationCtrl1->Play();
            }
        }
        if (!isAnimation) {
            _modelImage.LoadFile(fn.GetFullPath());
            if (_modelImage.IsOk()) {
                StaticBitmap_ModelImage->Show();
                AnimationCtrl1->Hide();
                LoadImage(StaticBitmap_ModelImage, &_modelImage);
            }
        }
    }
}

void VendorModelDialog::PopulateModelPanel(MModel* model)
{
    if (model == nullptr)
    {
        StaticBitmap_ModelImage->Hide();
        Button_Prior->Hide();
        Button_Next->Hide();
        StaticText5->Hide();
        TextCtrl_ModelDetails->Hide();
        HyperlinkCtrl_ModelWebLink->Hide();
        NotebookPanels->GetPage(1)->Hide();
        return;
    }

    model->DownloadImages();
    if (model->_imageFiles.size() > 0)
    {
        StaticBitmap_ModelImage->Show();
        Button_Prior->Show();
        Button_Next->Show();
        _currImage = 0;
        LoadModelImage(model->_imageFiles, _currImage);
    }
    else
    {
        StaticBitmap_ModelImage->Hide();
        Button_Prior->Hide();
        Button_Next->Hide();
    }

    TextCtrl_ModelDetails->Show();
    TextCtrl_ModelDetails->SetValue(model->GetDescription());

    if (!model->_webpage.empty())
    {
        StaticText5->Show();
        HyperlinkCtrl_ModelWebLink->Show();
        HyperlinkCtrl_ModelWebLink->SetURL(model->_webpage);
        HyperlinkCtrl_ModelWebLink->SetLabel("View model at " + ExtractURLServer(model->_webpage));
    }
    else
    {
        StaticText5->Hide();
        HyperlinkCtrl_ModelWebLink->Hide();
    }

    if (model->_wiring.size() == 0 || model->_wiring.front()->_xmodelLink.empty())
    {
        Button_InsertModel->Hide();
    }
    else
    {
        Button_InsertModel->Show();
    }

    Panel_Item->Layout();
}

void VendorModelDialog::PopulateModelPanel(MModelWiring* wiring)
{
    if (wiring == nullptr)
    {
        StaticBitmap_ModelImage->Hide();
        Button_Prior->Hide();
        Button_Next->Hide();
        StaticText5->Hide();
        TextCtrl_ModelDetails->Hide();
        HyperlinkCtrl_ModelWebLink->Hide();
        NotebookPanels->GetPage(1)->Hide();
        return;
    }

    wiring->DownloadImages();
    if (wiring->_imageFiles.size() > 0)
    {
        StaticBitmap_ModelImage->Show();
        Button_Prior->Show();
        Button_Next->Show();
        _currImage = 0;
        LoadModelImage(wiring->_imageFiles, _currImage);
    }
    else
    {
        StaticBitmap_ModelImage->Hide();
        Button_Prior->Hide();
        Button_Next->Hide();
    }

    TextCtrl_ModelDetails->Show();
    TextCtrl_ModelDetails->SetValue(wiring->GetDescription());

    if (!wiring->_model->_webpage.empty())
    {
        StaticText5->Show();
        HyperlinkCtrl_ModelWebLink->Show();
        HyperlinkCtrl_ModelWebLink->SetURL(wiring->_model->_webpage);
        HyperlinkCtrl_ModelWebLink->SetLabel("View model at " + ExtractURLServer(wiring->_model->_webpage));
    }
    else
    {
        StaticText5->Hide();
        HyperlinkCtrl_ModelWebLink->Hide();
    }

    if (wiring->_xmodelLink.empty())
    {
        Button_InsertModel->Hide();
    }
    else
    {
        Button_InsertModel->Show();
    }

    Panel_Item->Layout();
}

void VendorModelDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void VendorModelDialog::OnResize(wxSizeEvent& event)
{
    wxDialog::OnSize(event);

    if (NotebookPanels->GetSelection() == 0)
    {
        if (StaticBitmap_VendorImage->IsShown() && _vendorImage.IsOk())
        {
            if (_vendorImage.GetSize() != StaticBitmap_VendorImage->GetSize())
            {
                LoadImage(StaticBitmap_VendorImage, &_vendorImage);
            }
        }
    }
    else
    {
        if (StaticBitmap_ModelImage->IsShown() && _modelImage.IsOk())
        {
            if (_modelImage.GetSize() != StaticBitmap_ModelImage->GetSize())
            {
                LoadImage(StaticBitmap_ModelImage, &_modelImage);
            }
        }
    }

    ItemImagePanel->Refresh();
    PanelVendor->Refresh();
}

void VendorModelDialog::OnCheckBox_DontDownloadClick(wxCommandEvent& event)
{
    std::string vendor;
    wxTreeItemId startid = GetFocusedItem();

    if (GetFocusedItem().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(startid);

        if (tid != nullptr)
        {
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Vendor")
            {
                vendor = ((MVendorTreeItemData*)tid)->GetVendor()->_name;
            }
        }
    }

    if (vendor == "") return;

    if (!event.IsChecked())
    {
        wxMessageBox("Close and open the vendor download to load this vendor's model list.");
    }

    SuppressVendor(vendor, event.IsChecked());
}

bool VendorModelDialog::IsVendorSuppressed(const std::string& vendor)
{
    auto* config = GetXLightsConfig();

    std::string const suppress = config->Read("xLightsVendorSuppress", "DMX Fixture Library|");

    return Contains(suppress, vendor);
}

void VendorModelDialog::SuppressVendor(const std::string& vendor, bool suppress)
{
    auto* config = GetXLightsConfig();

    std::string s = config->Read("xLightsVendorSuppress", "DMX Fixture Library|");

    if (suppress && !Contains(s, vendor))
    {
        if (!EndsWith(s, "|")) s += "|";
        s += vendor;
    }
    else if (!suppress && Contains(s, vendor))
    {
        Replace(s, vendor, "");
        Replace(s, "||", "|");
    }
    config->Write("xLightsVendorSuppress", s);
}

void VendorModelDialog::OnTextCtrl_SearchText(wxCommandEvent& event)
{
	if (_lastSearchItem.IsOk()) {
		TreeCtrl_Navigator->SetItemBold(_lastSearchItem, false);
	}
	_lastSearchItem = wxTreeItemId();
	ValidateWindow();
}

void VendorModelDialog::OnSearchCancelClick(wxCommandEvent& event)
{
	if (_lastSearchItem.IsOk()) {
		TreeCtrl_Navigator->SetItemBold(_lastSearchItem, false);
	}
	_lastSearchItem = wxTreeItemId();
	TextCtrl_Search->SetValue("");
	TreeCtrl_Navigator->SetFocus();
	ValidateWindow();
}

void VendorModelDialog::OnButton_SearchClick(wxCommandEvent& event)
{
	// cant search if tree is empty
	if (TreeCtrl_Navigator->GetChildrenCount(TreeCtrl_Navigator->GetRootItem()) == 0)
	{
		wxBell();
		return;
	}

	// Prefer the item we last landed on, so repeated presses advance through results.
	// GetSelection() is undefined for wxTR_MULTIPLE trees; GetFocusedItem() loses its
	// value when the tree doesn't have application focus (Windows). A member variable
	// is the only reliable way to track the search cursor.
	wxTreeItemId current = _lastSearchItem;
	if (!current.IsOk()) {
		current = GetFocusedItem();
	}
	if (!current.IsOk()) {
		current = TreeCtrl_Navigator->GetRootItem();
	}
	wxTreeItemId start = current;
	if (current.IsOk())
	{
		do
		{
			// if this node has children got to the first child
			if (TreeCtrl_Navigator->GetChildrenCount(current, false) > 0)
			{
				wxTreeItemIdValue cookie;
				current = TreeCtrl_Navigator->GetFirstChild(current, cookie);
			}
			else
			{
				// no child ... so go to the sibling
				auto sibling = TreeCtrl_Navigator->GetNextSibling(current);
				if (sibling.IsOk())
				{
					current = sibling;
				}
				else
				{
					// no sibling so we need to move up until a sibling exists and then get the next one.
					for (;;)
					{
						auto parent = TreeCtrl_Navigator->GetItemParent(current);
						if (parent == TreeCtrl_Navigator->GetRootItem())
						{
							current = parent;
							break;
						}
						else if (parent.IsOk())
						{
							sibling = TreeCtrl_Navigator->GetNextSibling(parent);
							if (sibling.IsOk())
							{
								current = sibling;
								break;
							}
							else
							{
								current = parent;
								if (current == TreeCtrl_Navigator->GetRootItem())
								{
									break;
								}
							}
						}
						else
						{
							wxBell();
							return;
						}
					}
				}
			}

			if (current != TreeCtrl_Navigator->GetRootItem() && TreeCtrl_Navigator->GetItemText(current).Lower().Contains(TextCtrl_Search->GetValue().Lower()))
			{
				// Bold the found item for visual feedback without touching the
				// selection — SelectItem on wxTR_MULTIPLE clears all other selections
				// on Windows, which would wipe out the user's Ctrl+click choices.
				if (_lastSearchItem.IsOk()) {
					TreeCtrl_Navigator->SetItemBold(_lastSearchItem, false);
				}
				_lastSearchItem = current;
				TreeCtrl_Navigator->SetItemBold(current, true);
				TreeCtrl_Navigator->EnsureVisible(current);
				UpdatePanelForItem(current);
				if (current == start)
				{
					wxBell();
				}
				return;
			}

		} while (current != start);
		wxBell();
	}
}
