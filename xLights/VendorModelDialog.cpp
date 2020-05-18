/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VendorModelDialog.h"

//(*InternalHeaders(VendorModelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <log4cpp/Category.hh>
#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include "CachedFileDownloader.h"
#include "UtilFunctions.h"

CachedFileDownloader VendorModelDialog::_cache;

class MModel;

class MModelWiring
{
public:
    std::list<wxURI> _images;
    std::list<wxFileName> _imageFiles;
    std::string _name;
    std::string _wiringDescription;
    wxURI _xmodelLink;
    wxFileName _xmodelFile;
    MModel* _model;

    std::string GetDescription();

    ~MModelWiring() { }

    wxColor GetColour() const
    {
        if (_xmodelLink.BuildURI() == "") {
            return wxColour(255, 128, 0);
        }
        else {
            return *wxBLUE;
        }
    }

    void DownloadImages()
    {
        if (_imageFiles.size() != _images.size()) {
            _imageFiles.clear();
            for (const auto& it : _images) {
                std::string fn = VendorModelDialog::GetCache().GetFile(it, CACHEFOR::CACHETIME_LONG);
                if (fn != "") {
                    _imageFiles.push_back(wxFileName(fn));
                }
            }
        }
    }

    void DownloadXModel();

    MModelWiring(wxXmlNode* n, MModel* m)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        _model = m;

        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext()) {
            if (l->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
                wxString nn = l->GetName().Lower().ToStdString();
                if (nn == "name") {
                    _name = l->GetNodeContent().ToStdString();
                }
                else if (nn == "description") {
                    _wiringDescription = l->GetNodeContent().ToStdString();
                }
                else if (nn == "xmodellink") {
                    _xmodelLink = wxURI(l->GetNodeContent());
                }
                else if (nn == "imagefile") {
                    _images.push_back(wxURI(l->GetNodeContent()));
                }
                else {
                    logger_base.warn("MModelWiring: Error processing vendor xml: %s ", (const char*)nn.c_str());
                    wxASSERT(false);
                }
            }
        }
    }

    void AddImages(std::list<wxURI> images)
    {
        for (const auto& it : images) {
            _images.push_back(it);
        }
    }
};

class MModel
{
public:
    std::string _id;
    std::list<std::string> _categoryIds;
    std::string _name;
    std::string _type;
    std::string _material;
    std::string _thickness;
    std::string _width;
    std::string _height;
    std::string _pixelCount;
    std::string _pixelSpacing;
    std::string _pixelDescription;
    wxURI _webpage;
    std::list<wxURI> _images;
    std::list<wxFileName> _imageFiles;
    std::string _notes;
    std::list<MModelWiring*> _wiring;
    MVendor* _vendor = nullptr;

    bool InCategory(std::string category)
    {
        for (const auto& it : _categoryIds) {
            if (it == category) return true;
        }

        return false;
    }

    wxColor GetColour() const
    {
        if (_wiring.size() == 0 || _wiring.front()->_xmodelLink.BuildURI() == "") {
            return wxColour(255, 128, 0);
        }
        else {
            return *wxBLUE;
        }
    }

    MModel(wxXmlNode* n, MVendor* vendor)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        _vendor = vendor;

        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext()) {
            if (l->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
                wxString nn = l->GetName().Lower().ToStdString();
                if (nn == "id") {
                    _id = l->GetNodeContent().ToStdString();
                }
                else if (nn == "categoryid") {
                    _categoryIds.push_back(l->GetNodeContent().ToStdString());
                }
                else if (nn == "name") {
                    _name = l->GetNodeContent().ToStdString();
                }
                else if (nn == "type") {
                    _type = l->GetNodeContent().ToStdString();
                }
                else if (nn == "material") {
                    _material = l->GetNodeContent().ToStdString();
                }
                else if (nn == "thickness") {
                    _thickness = l->GetNodeContent().ToStdString();
                }
                else if (nn == "width") {
                    _width = l->GetNodeContent().ToStdString();
                }
                else if (nn == "height") {
                    _height = l->GetNodeContent().ToStdString();
                }
                else if (nn == "pixelcount") {
                    _pixelCount = l->GetNodeContent().ToStdString();
                }
                else if (nn == "pixelspacing") {
                    _pixelSpacing = l->GetNodeContent().ToStdString();
                }
                else if (nn == "pixeldescription") {
                    _pixelDescription = l->GetNodeContent().ToStdString();
                }
                else if (nn == "notes") {
                    _notes = l->GetNodeContent().ToStdString();
                }
                else if (nn == "weblink") {
                    _webpage = wxURI(l->GetNodeContent());
                }
                else if (nn == "imagefile") {
                    _images.push_back(wxURI(l->GetNodeContent()));
                }
                else if (nn == "wiring") {
                    _wiring.push_back(new MModelWiring(l, this));
                }
                else {
                    logger_base.warn("MModel: Error processing vendor xml: %s ", (const char*)nn.c_str());
                    wxASSERT(false);
                }
            }
        }

        for (const auto& it : _wiring) {
            it->AddImages(_images);
        }
    }

    virtual ~MModel()
    {
        for (auto& it : _wiring)
        {
            delete it;
        }
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
            desc += PadTitle("Name:") + _name + "\n\n";
        }
        if (_type != "") {
            desc += PadTitle("Type:") + _type + "\n";
        }
        if (_material != "") {
            desc += PadTitle("Material:") + _material + "\n";
        }
        if (_thickness != "") {
            desc += PadTitle("Thickness:") + _thickness + "\n";
        }
        if (_width != "") {
            desc += PadTitle("Width:") + _width + "\n";
        }
        if (_height != "") {
            desc += PadTitle("Height:") + _height + "\n";
        }
        if (_pixelCount != "") {
            desc += PadTitle("Pixel Count:") + _pixelCount + "\n";
        }
        if (_pixelSpacing != "") {
            desc += PadTitle("Minimum Pixel Spacing:") + _pixelSpacing + "\n";
        }
        if (_pixelDescription != "") {
            desc += PadTitle("Pixel Description:") + _pixelDescription + "\n";
        }
        if (_notes != "") {
            desc += "\n" + _notes + "\n";
        }

        return desc;
    }

    void DownloadImages()
    {
        if (_imageFiles.size() == _images.size()) {
            return;
        }
        _imageFiles.clear();

        for (auto it = _images.begin(); it != _images.end(); ++it) {
            std::string fn = VendorModelDialog::GetCache().GetFile(*it, CACHEFOR::CACHETIME_LONG);
            if (fn != "") {
                _imageFiles.push_back(wxFileName(fn));
            }
        }
    }
};

void MModelWiring::DownloadXModel()
{
    if (!_xmodelFile.Exists()) {
        _xmodelFile = VendorModelDialog::GetCache().GetFile(_xmodelLink, CACHEFOR::CACHETIME_LONG);

        wxXmlDocument d;
        d.Load(_xmodelFile.GetFullPath());
        if (d.IsOk()) {
            wxXmlNode* root = d.GetRoot();
            if (root->GetAttribute("PixelType", "") == "" && _model->_pixelDescription != "") {
                root->AddAttribute("PixelType", _model->_pixelDescription);
            }
            if (root->GetAttribute("PixelMinimumSpacingInches", "") == "" && wxAtoi(_model->_pixelSpacing) != 0) {
                root->AddAttribute("PixelMinimumSpacingInches", wxString::Format("%d", wxAtoi(_model->_pixelSpacing)));
            }
            if (root->GetAttribute("PixelCount", "") == "" && wxAtoi(_model->_pixelCount) != 0) {
                root->AddAttribute("PixelCount", wxString::Format("%d", wxAtoi(_model->_pixelCount)));
            }
            d.Save(_xmodelFile.GetFullPath());
        }
    }
}

std::string MModelWiring::GetDescription()
{
    std::string desc = _model->GetDescription();
    desc += "\n";

    if (_name != "") {
        desc += "Wiring Option: " + _name + "\n\n";
    }
    if (_wiringDescription != "") {
        desc += _wiringDescription;
    }

    return desc;
}


class MVendorCategory
{
    void ParseCategories(wxXmlNode* n)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext()) {
            if (l->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
                wxString nn = l->GetName().Lower().ToStdString();
                if (nn == "category") {
                    _categories.push_back(new MVendorCategory(l, this, _vendor));

                }
                else {
                    logger_base.warn("MVendorCategory: Error processing vendor categories xml: %s ", (const char*)nn.c_str());
                }
            }
        }
    }

    public:
        std::string _id;
        std::string _name;
        MVendorCategory* _parent;
        std::list<MVendorCategory*> _categories;
        MVendor* _vendor;

        std::string GetPath() const
        {
            if (_parent != nullptr) {
                return _parent->GetPath() + "/" + _name;
            }
            else {
                return _name;
            }
        }

    MVendor* GetVendor() const { return _vendor; }

    MVendorCategory(wxXmlNode* n, MVendorCategory* parent, MVendor* vendor);
    virtual ~MVendorCategory()
    {
        for (auto& it : _categories) {
            delete it;
        }
    }
};

class MVendor
{
public:
    std::string _name = "";
    std::string _contact = "";
    std::string _email = "";
    std::string _phone = "";
    wxURI _website;
    wxURI _facebook;
    std::string _twitter = "";
    std::string _notes = "";
    wxFileName _logoFile;
    std::list<MVendorCategory*> _categories;
    std::list<MModel*> _models;
    int _maxModels = 0;

    std::list<MModel*> GetModels(std::string categoryId)
    {
        std::list<MModel*> res;

        for (const auto& it : _models) {
            if (it->InCategory(categoryId)) {
                res.push_back(it);
            }
        }

        return res;
    }

    void ParseCategories(wxXmlNode* n)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext()) {
            if (l->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
                wxString nn = l->GetName().Lower().ToStdString();
                if (nn == "category") {
                    _categories.push_back(new MVendorCategory(l, nullptr, this));
                }
                else {
                    logger_base.warn("MVendor: Error processing vendor categories xml: %s ", (const char*)nn.c_str());
                    wxASSERT(false);
                }
            }
        }
    }

    std::string PadTitle(std::string t) const
    {
        std::string res = t;
        while (res.size() < 9) res += " ";
        return res;
    }

    std::string GetDescription() const
    {
        std::string desc;

        if (_name != "") {
            desc += PadTitle("Name:") + _name + "\n\n";
        }
        if (_contact != "") {
            desc += PadTitle("Contact:") + _contact + "\n";
        }
        if (_phone != "") {
            desc += PadTitle("Phone:") + _phone + "\n";
        }
        if (_email != "") {
            desc += PadTitle("Email:") + _email + "\n";
        }
        if (_twitter != "") {
            desc += PadTitle("Twitter:") + _twitter + "\n";
        }
        if (_notes != "") {
            desc += "\n" + _notes + "\n";
        }

        return desc;
    }

    MVendor(const std::string& name)
    {
        _name = name;
    }

    MVendor(wxXmlDocument* doc, int maxModels)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        _maxModels = maxModels;

        if (doc->IsOk()) {
            wxXmlNode* root = doc->GetRoot();
            wxString nn = root->GetName().Lower();
            if (nn == "modelinventory") {
                for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
                    if (e->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
                        nn = e->GetName().Lower();
                        if (nn == "vendor") {
                            for (wxXmlNode* v = e->GetChildren(); v != nullptr; v = v->GetNext()) {
                                if (v->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
                                    nn = v->GetName().Lower();
                                    if (nn == "name") {
                                        _name = v->GetNodeContent().ToStdString();
                                    }
                                    else if (nn == "contact") {
                                        _contact = v->GetNodeContent().ToStdString();
                                    }
                                    else if (nn == "email") {
                                        _email = v->GetNodeContent().ToStdString();
                                    }
                                    else if (nn == "phone") {
                                        _phone = v->GetNodeContent().ToStdString();
                                    }
                                    else if (nn == "website") {
                                        _website = wxURI(v->GetNodeContent().ToStdString());
                                    }
                                    else if (nn == "facebook") {
                                        _facebook = wxURI(v->GetNodeContent().ToStdString());
                                    }
                                    else if (nn == "twitter") {
                                        _twitter = v->GetNodeContent().ToStdString();
                                    }
                                    else if (nn == "notes") {
                                        _notes = v->GetNodeContent().ToStdString();
                                    }
                                    else if (nn == "logolink") {
                                        wxURI logo(v->GetNodeContent().ToStdString());
                                        _logoFile = wxFileName(VendorModelDialog::GetCache().GetFile(logo, CACHEFOR::CACHETIME_LONG));
                                    }
                                    else {
                                        logger_base.warn("MVendor: Error processing vendor xml: %s ", (const char*)nn.c_str());
                                        wxASSERT(false);
                                    }
                                }
                            }
                        }
                        else if (nn == "categories") {
                            ParseCategories(e);
                        }
                        else if (nn == "models") {
                            int models = 0;
                            for (wxXmlNode* m = e->GetChildren(); m != nullptr; m = m->GetNext()) {
                                nn = m->GetName().Lower();
                                if (nn == "model") {
                                    models++;
                                    if (maxModels < 1 || models < _maxModels) {
                                        _models.push_back(new MModel(m, this));
                                    }
                                }
                            }
                        }
                        else {
                            logger_base.warn("MVendor: Error processing vendor xml: %s ", (const char*)nn.c_str());
                            wxASSERT(false);
                        }
                    }
                }
            }
        }
    }

    virtual ~MVendor()
    {
        for (auto& it : _categories)
        {
            delete it;
        }

        for (auto& it : _models)
        {
            delete it;
        }
    }
};

MVendorCategory::MVendorCategory(wxXmlNode* n, MVendorCategory* parent, MVendor* vendor)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    _vendor = vendor;
    _parent = parent;
    for (wxXmlNode* e = n->GetChildren(); e != nullptr; e = e->GetNext()) {
        // ignore comment nodes
        if (e->GetType() != wxXmlNodeType::wxXML_COMMENT_NODE) {
            wxString nn = e->GetName().Lower();
            if (nn == "id") {
                _id = e->GetNodeContent().ToStdString();
            }
            else if (nn == "name") {
                _name = e->GetNodeContent().ToStdString();
            }
            else if (nn == "categories") {
                ParseCategories(e);
            }
            else {
                logger_base.warn("MVendorCategory: Error processing vendor xml: %s : %s : %s : %s", (const char*)vendor->_name.c_str(), (const char*)parent->_name.c_str(), (const char*)nn.c_str(), (const char*)GetPath().c_str());
                wxASSERT(false);
            }
        }
    }
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
const long VendorModelDialog::ID_TREECTRL1 = wxNewId();
const long VendorModelDialog::ID_PANEL3 = wxNewId();
const long VendorModelDialog::ID_CHECKBOX1 = wxNewId();
const long VendorModelDialog::ID_STATICBITMAP1 = wxNewId();
const long VendorModelDialog::ID_TEXTCTRL1 = wxNewId();
const long VendorModelDialog::ID_STATICTEXT8 = wxNewId();
const long VendorModelDialog::ID_HYPERLINKCTRL4 = wxNewId();
const long VendorModelDialog::ID_STATICTEXT4 = wxNewId();
const long VendorModelDialog::ID_HYPERLINKCTRL2 = wxNewId();
const long VendorModelDialog::ID_PANEL2 = wxNewId();
const long VendorModelDialog::ID_BUTTON2 = wxNewId();
const long VendorModelDialog::ID_STATICBITMAP2 = wxNewId();
const long VendorModelDialog::ID_BUTTON3 = wxNewId();
const long VendorModelDialog::ID_PANEL5 = wxNewId();
const long VendorModelDialog::ID_TEXTCTRL2 = wxNewId();
const long VendorModelDialog::ID_STATICTEXT7 = wxNewId();
const long VendorModelDialog::ID_HYPERLINKCTRL3 = wxNewId();
const long VendorModelDialog::ID_BUTTON1 = wxNewId();
const long VendorModelDialog::ID_PANEL4 = wxNewId();
const long VendorModelDialog::ID_NOTEBOOK1 = wxNewId();
const long VendorModelDialog::ID_PANEL1 = wxNewId();
const long VendorModelDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VendorModelDialog,wxDialog)
	//(*EventTable(VendorModelDialog)
	//*)
END_EVENT_TABLE()

VendorModelDialog::VendorModelDialog(wxWindow* parent, const std::string& showFolder, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    _showFolder = showFolder;

    //(*Initialize(VendorModelDialog)
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
    CheckBox_DontDownload = new wxCheckBox(PanelVendor, ID_CHECKBOX1, _("Don\'t download this vendors list of models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_DontDownload->SetValue(false);
    FlexGridSizer4->Add(CheckBox_DontDownload, 1, wxALL | wxEXPAND, 5);
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
    HyperlinkCtrl_Facebook = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL4, _("http://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL4"));
    FlexGridSizer5->Add(HyperlinkCtrl_Facebook, 1, wxALL | wxEXPAND, 5);
    StaticText2 = new wxStaticText(PanelVendor, ID_STATICTEXT4, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer5->Add(StaticText2, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_Website = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL2, _("http://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL2"));
    FlexGridSizer5->Add(HyperlinkCtrl_Website, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL | wxEXPAND, 5);
    PanelVendor->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(PanelVendor);
    FlexGridSizer4->SetSizeHints(PanelVendor);
    Panel_Item = new wxPanel(NotebookPanels, ID_PANEL4, wxPoint(41, 9), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6->AddGrowableCol(0);
    FlexGridSizer6->AddGrowableRow(1);
    ItemImagePanel = new wxPanel(Panel_Item, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer7->AddGrowableCol(1);
    FlexGridSizer7->AddGrowableRow(0);
    Button_Prior = new wxButton(ItemImagePanel, ID_BUTTON2, _("<"), wxDefaultPosition, wxSize(30, -1), 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer7->Add(Button_Prior, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    StaticBitmap_ModelImage = new wxStaticBitmap(ItemImagePanel, ID_STATICBITMAP2, wxNullBitmap, wxDefaultPosition, wxSize(256, 256), wxSIMPLE_BORDER, _T("ID_STATICBITMAP2"));
    StaticBitmap_ModelImage->SetMinSize(wxSize(256, 256));
    FlexGridSizer7->Add(StaticBitmap_ModelImage, 1, wxALL | wxEXPAND, 5);
    Button_Next = new wxButton(ItemImagePanel, ID_BUTTON3, _(">"), wxDefaultPosition, wxSize(30, -1), 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer7->Add(Button_Next, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    ItemImagePanel->SetSizer(FlexGridSizer7);
    FlexGridSizer7->Fit(ItemImagePanel);
    FlexGridSizer7->SetSizeHints(ItemImagePanel);
    FlexGridSizer6->Add(ItemImagePanel, 1, wxALL | wxEXPAND, 0);
    TextCtrl_ModelDetails = new wxTextCtrl(Panel_Item, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
    FlexGridSizer6->Add(TextCtrl_ModelDetails, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer8->AddGrowableCol(1);
    StaticText5 = new wxStaticText(Panel_Item, ID_STATICTEXT7, _("Web Link:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer8->Add(StaticText5, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
    HyperlinkCtrl_ModelWebLink = new wxHyperlinkCtrl(Panel_Item, ID_HYPERLINKCTRL3, _("http://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU | wxHL_ALIGN_LEFT | wxNO_BORDER, _T("ID_HYPERLINKCTRL3"));
    FlexGridSizer8->Add(HyperlinkCtrl_ModelWebLink, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer6->Add(FlexGridSizer8, 1, wxALL | wxEXPAND, 5);
    Button_InsertModel = new wxButton(Panel_Item, ID_BUTTON1, _("Insert Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer6->Add(Button_InsertModel, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
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

    Connect(ID_TREECTRL1, wxEVT_COMMAND_TREE_ITEM_ACTIVATED, (wxObjectEventFunction)&VendorModelDialog::OnTreeCtrl_NavigatorItemActivated);
    Connect(ID_TREECTRL1, wxEVT_COMMAND_TREE_SEL_CHANGED, (wxObjectEventFunction)&VendorModelDialog::OnTreeCtrl_NavigatorSelectionChanged);
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

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("File cache size: %d", _cache.size());

    PopulateModelPanel((MModel*)nullptr);
    PopulateVendorPanel(nullptr);

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

wxXmlDocument* VendorModelDialog::GetXMLFromURL(wxURI url, std::string& filename, wxProgressDialog* prog, int low, int high) const
{
    filename = "";
    wxFileName fn = wxFileName(VendorModelDialog::GetCache().GetFile(url, CACHEFOR::CACHETIME_SESSION, "", prog, low, high));
    if (fn.Exists()) {
        filename = fn.GetFullPath();
        return new wxXmlDocument(filename);
    }

    return nullptr;
}

bool VendorModelDialog::LoadTree(wxProgressDialog* prog, int low, int high)
{
    const std::string vendorlink = "http://nutcracker123.com/xlights/vendors/xlights_vendors.xml";
    //const std::string vendorlink = "http://threebuttes.com/Extras/dmx_models/xlights_vendors.xml";

    std::string filename;
    wxXmlDocument* vd = GetXMLFromURL(wxURI(vendorlink), filename, prog, low, high);
    if (vd != nullptr && vd->IsOk())
    {
        wxXmlNode* root = vd->GetRoot();

        for (auto v = root->GetChildren(); v != nullptr; v = v->GetNext())
        {
            if (v->GetName().Lower() == "vendor")
            {
                int maxModels = -1;
                std::string url = "";
                std::string name = "";

                for (auto link = v->GetChildren(); link != nullptr; link = link->GetNext())
                {
                    if (link->GetName().Lower() == "link")
                    {
                        url = link->GetNodeContent().ToStdString();
                    }
                    else if (link->GetName().Lower() == "maxmodels")
                    {
                        maxModels = wxAtoi(link->GetNodeContent());
                    }
                    else if (link->GetName().Lower() == "name")
                    {
                        name =link->GetNodeContent();
                    }
                }

                if (name != "" && IsVendorSuppressed(name))
                {
                    MVendor* mv = new MVendor(name);
                    _vendors.push_back(mv);
                }
                else
                {
                    if (url != "")
                    {
                        std::string vfilename;
                        wxXmlDocument* d = GetXMLFromURL(wxURI(url), vfilename, prog, low, high);
                        if (d != nullptr && d->IsOk())
                        {
                            MVendor* mv = new MVendor(d, maxModels);
                            _vendors.push_back(mv);
                            delete d;
                        }
                    }
                }
            }
        }
    }
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
        TreeCtrl_Navigator->Expand(v);
    }

    if (first.IsOk() && first != root)
    {
        TreeCtrl_Navigator->EnsureVisible(first);
    }

    wxTreeItemIdValue cookie;
    for (auto l1 = TreeCtrl_Navigator->GetFirstChild(root, cookie); l1.IsOk(); l1 = TreeCtrl_Navigator->GetNextChild(root, cookie))
    {
        DeleteEmptyCategories(l1);
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
            DeleteEmptyCategories(l1);
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
                TreeCtrl_Navigator->SetItemTextColour(id, it->GetColour());
            }
        }
        else
        {
            if (it->_wiring.size() == 0)
            {
                wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(v, it->_name, -1, -1, new MModelTreeItemData(it));
                TreeCtrl_Navigator->SetItemTextColour(tid, it->GetColour());
            }
            else
            {
                wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(v, it->_name, -1, -1, new MWiringTreeItemData(it->_wiring.front()));
                TreeCtrl_Navigator->SetItemTextColour(tid, it->GetColour());
            }
        }
    }
}

VendorModelDialog::~VendorModelDialog()
{
	//(*Destroy(VendorModelDialog)
	//*)

    _cache.Save();

    for (auto it = _vendors.begin(); it != _vendors.end(); ++it)
    {
        delete *it;
    }
}

void VendorModelDialog::OnHyperlinkCtrl_ModelWebLinkClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(HyperlinkCtrl_ModelWebLink->GetURL());
}

void VendorModelDialog::OnButton_PriorClick(wxCommandEvent& event)
{
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

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
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

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

void VendorModelDialog::OnButton_InsertModelClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr && ((VendorBaseTreeItemData*)tid)->GetType() == "Wiring")
        {
            ((MWiringTreeItemData*)tid)->GetWiring()->DownloadXModel();
            if (((MWiringTreeItemData*)tid)->GetWiring()->_xmodelFile.GetExt().Lower() == "zip")
            {
                // we need to open the zip ... place the files in the "modeldownload" folder in the show folder
                logger_base.debug("    opening zipped model " + _modelFile);

                auto dir = _showFolder + wxFileName::GetPathSeparator() + "modeldownload";
                if (!wxDir::Exists(dir))
                {
                    logger_base.debug("Creating modeldownload directory " + dir);
                    wxMkdir(dir);
                }

                // files should be .jpg/png/bmp/obj ... and one xmodel file
                wxFileInputStream fin(((MWiringTreeItemData*)tid)->GetWiring()->_xmodelFile.GetFullPath());
                if (fin.IsOk())
                {
                    wxZipInputStream zin(fin);
                    if (zin.IsOk())
                    {
                        wxZipEntry* ent = zin.GetNextEntry();
                        while (ent != nullptr) {

                            // create any needed subfolders
                            if (ent->GetName().Contains('\\') || ent->GetName().Contains('/'))
                            {
                                wxArrayString aa = wxSplit(ent->GetName(), '\\');
                                wxArrayString bb = wxSplit(ent->GetName(), '/');

                                auto createdirs = [](const wxString& parent, const wxString& sub) {
                                    auto d = parent + wxFileName::GetPathSeparator() + sub;
                                    if (!wxDir::Exists(d))
                                    {
                                        logger_base.debug("Creating modeldownload subdirectory " + d);
                                        wxMkdir(d);
                                    }
                                    return d;
                                };

                                wxString parent = dir;
                                for (int i = 0; i < aa.size() - 1; i++)
                                {
                                    parent = createdirs(parent, aa[i]);
                                }

                                parent = dir;
                                for (int i = 0; i < bb.size() - 1; i++)
                                {
                                    parent = createdirs(parent, bb[i]);
                                }
                            }

                            auto file = dir + wxFileName::GetPathSeparator() + ent->GetName();
                            if (wxFileName(ent->GetName()).GetExt().Lower() == "xmodel") {
                                _modelFile = file;
                            }

                            if (!wxFile::Exists(file))
                            {
                                logger_base.debug("        model file " + file + " downloaded.");
                                wxFileOutputStream fout(file);
                                zin.Read(fout);
                            }
                            else
                            {
                                logger_base.warn("        skipping file " + file + " it already exists.");
                            }

                            ent = zin.GetNextEntry();
                        }
                    }
                    else { 
                        logger_base.error("Failed to open zip file.");
                        return; }
                }
                else { 
                    logger_base.error("Failed to open zip file.");
                    return; }
            }
            else
            {
                _modelFile = ((MWiringTreeItemData*)tid)->GetWiring()->_xmodelFile.GetFullPath();
            }
        }
    }

    EndDialog(wxID_OK);
}

void VendorModelDialog::OnNotebookPanelsPageChanged(wxNotebookEvent& event)
{
}

void VendorModelDialog::OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event)
{
    ValidateWindow();
}

void VendorModelDialog::OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event)
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
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Vendor")
            {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->GetPage(1)->Hide();
                NotebookPanels->SetSelection(0);
                PopulateVendorPanel(((MVendorTreeItemData*)tid)->GetVendor());
                PopulateModelPanel((MModel*)nullptr);
                TreeCtrl_Navigator->SetFocus();
            }
            else if (type == "Model")
            {
                NotebookPanels->GetPage(0)->Hide();
                NotebookPanels->GetPage(1)->Show();
                NotebookPanels->SetSelection(1);
                TreeCtrl_Navigator->SetFocus();
                PopulateModelPanel(((MModelTreeItemData*)tid)->GetModel());
                PopulateVendorPanel(((MModelTreeItemData*)tid)->GetModel()->_vendor);
            }
            else if (type == "Wiring")
            {
                NotebookPanels->GetPage(0)->Hide();
                NotebookPanels->GetPage(1)->Show();
                NotebookPanels->SetSelection(1);
                TreeCtrl_Navigator->SetFocus();
                PopulateModelPanel(((MWiringTreeItemData*)tid)->GetWiring());
                PopulateVendorPanel(((MWiringTreeItemData*)tid)->GetWiring()->_model->_vendor);
            }
            else
            {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->GetPage(1)->Hide();
                NotebookPanels->SetSelection(0);
                PopulateVendorPanel(((MCategoryTreeItemData*)tid)->GetCategory()->GetVendor());
                PopulateModelPanel((MModel*)nullptr);
                TreeCtrl_Navigator->SetFocus();
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

    if (startid != TreeCtrl_Navigator->GetSelection())
    {
        // selection changed while we were processing so lets try again
        wxPostEvent(this, event);
    }
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
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr)
        {
            int imageCount = 0;
            std::string type = ((VendorBaseTreeItemData*)tid)->GetType();

            if (type == "Wiring")
            {
                Button_InsertModel->Enable();
                imageCount = ((MWiringTreeItemData*)tid)->GetWiring()->_imageFiles.size();
            }
            else
            {
                Button_InsertModel->Disable();
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
        Button_InsertModel->Disable();
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

    if (vendor->_logoFile.Exists())
    {
        _vendorImage.LoadFile(vendor->_logoFile.GetFullPath());
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

    if (vendor->_facebook.GetPath() != "")
    {
        StaticText6->Show();
        HyperlinkCtrl_Facebook->Show();
        HyperlinkCtrl_Facebook->SetURL(vendor->_facebook.BuildURI());
        HyperlinkCtrl_Facebook->SetLabel(vendor->_facebook.BuildURI());
    }
    else
    {
        StaticText6->Hide();
        HyperlinkCtrl_Facebook->Hide();
    }
    if (vendor->_website.GetPath() != "")
    {
        StaticText2->Show();
        HyperlinkCtrl_Website->Show();
        HyperlinkCtrl_Website->SetURL(vendor->_website.BuildURI());
        HyperlinkCtrl_Website->SetLabel(vendor->_website.BuildURI());
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

void VendorModelDialog::LoadModelImage(std::list<wxFileName> imageFiles, int image)
{
    auto it = imageFiles.begin();
    for (int i = 0; i < image; i++)
    {
        ++it;
    }
    if (it->Exists())
    {
        _modelImage.LoadFile(it->GetFullPath());
        if (_modelImage.IsOk())
        {
            LoadImage(StaticBitmap_ModelImage, &_modelImage);
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

    if (model->_webpage.GetPath() != "")
    {
        StaticText5->Show();
        HyperlinkCtrl_ModelWebLink->Show();
        HyperlinkCtrl_ModelWebLink->SetURL(model->_webpage.BuildURI());
        HyperlinkCtrl_ModelWebLink->SetLabel("View model at " + model->_webpage.GetServer());
    }
    else
    {
        StaticText5->Hide();
        HyperlinkCtrl_ModelWebLink->Hide();
    }

    if (model->_wiring.size() == 0 || model->_wiring.front()->_xmodelLink.BuildURI() == "")
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

    if (wiring->_model->_webpage.GetPath() != "")
    {
        StaticText5->Show();
        HyperlinkCtrl_ModelWebLink->Show();
        HyperlinkCtrl_ModelWebLink->SetURL(wiring->_model->_webpage.BuildURI());
        HyperlinkCtrl_ModelWebLink->SetLabel("View model at " + wiring->_model->_webpage.GetServer());
    }
    else
    {
        StaticText5->Hide();
        HyperlinkCtrl_ModelWebLink->Hide();
    }

    if (wiring->_xmodelLink.BuildURI() == "")
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
    std::string vendor = "";
    wxTreeItemId startid = TreeCtrl_Navigator->GetSelection();

    if (TreeCtrl_Navigator->GetSelection().IsOk())
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
    wxConfigBase* config = wxConfigBase::Get();

    auto suppress = config->Read("xLightsVendorSuppress", "DMX Fixture Library|");

    return suppress.Contains(vendor);
}

void VendorModelDialog::SuppressVendor(const std::string& vendor, bool suppress)
{
    wxConfigBase* config = wxConfigBase::Get();

    auto s = config->Read("xLightsVendorSuppress", "|DMX Fixture Library");

    if (suppress && !s.Contains(vendor))
    {
        if (!s.EndsWith('|')) s += "|";
        s += vendor;
    }
    else if (!suppress && s.Contains(vendor))
    {
        s.Replace(vendor, "");
        s.Replace("||", "|");
    }
    config->Write("xLightsVendorSuppress", s);
}
