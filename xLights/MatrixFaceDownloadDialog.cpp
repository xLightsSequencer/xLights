#include "MatrixFaceDownloadDialog.h"

//(*InternalHeaders(MatrixFaceDownloadDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include "CachedFileDownloader.h"
#include <log4cpp/Category.hh>

CachedFileDownloader MatrixFaceDownloadDialog::_cache;

class MFace;

class MFace
{
public:
    wxURI _image;
    wxFileName _imageFile;
    std::string _name;
    std::string _description;
    std::string _artist;
    std::string _copyright;
    wxURI _faceLink;
    wxFileName _faceFile;
    std::string _id;
    std::list<std::string> _categoryIds;
    int _width;
    int _height;
    int _minwidth;
    int _minheight;

    std::string GetDescription();

    ~MFace()
    {
    }

    void DownloadImages()
    {
		std::string fn = MatrixFaceDownloadDialog::GetCache().GetFile(_image, CACHEFOR::CACHETIME_LONG);
		if (fn != "")
		{
			_imageFile = wxFileName(fn);
		}
    }

    void DownloadFile();

    MFace(wxXmlNode* n)
    {
		_width = -1;
		_height = -1;
		_minwidth = -1;
		_minheight = -1;
		
        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
        {
            wxString nn = l->GetName().Lower().ToStdString();
            if (nn == "name")
            {
                _name = l->GetNodeContent().ToStdString();
            }
            else if (nn == "description")
            {
				_description = l->GetNodeContent().ToStdString();
            }
            else if (nn == "artist")
            {
				_artist = l->GetNodeContent().ToStdString();
            }
            else if (nn == "copyright")
            {
				_copyright = l->GetNodeContent().ToStdString();
            }
            else if (nn == "facelink")
            {
                _faceLink = wxURI(l->GetNodeContent());
            }
            else if (nn == "faceimage")
            {
                _image = wxURI(l->GetNodeContent());
            }
            else if (nn == "id")
            {
                _id = l->GetNodeContent().ToStdString();
            }
            else if (nn == "categoryid")
            {
                _categoryIds.push_back(l->GetNodeContent().ToStdString());
            }
            else if (nn == "width")
            {
                _width = wxAtoi(l->GetNodeContent());
            }
            else if (nn == "height")
            {
                _height = wxAtoi(l->GetNodeContent());
            }
            else if (nn == "minwidth")
            {
                _minwidth = wxAtoi(l->GetNodeContent());
            }
            else if (nn == "minheight")
            {
                _minheight = wxAtoi(l->GetNodeContent());
            }
            else if (nn == "notes")
            {
                _notes = l->GetNodeContent().ToStdString();
            }
            else
            {
                wxASSERT(false);
            }
        }
    }

	bool InCategory(std::string category)
    {
        for (auto it = _categoryIds.begin(); it != _categoryIds.end(); ++it)
        {
            if (*it == category) return true;
        }

        return false;
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

        if (_name != "")
        {
            desc += PadTitle("Name:") + _name + "\n\n";
        }
        if (_artist != "")
        {
            desc += PadTitle("Artist:") + _artist + "\n\n";
        }
        if (_copyright != "")
        {
            desc += PadTitle("Copyright:") + _copyright + "\n\n";
        }
		if (_width != -1)
		{
			desc += PadTitle("Width:") + wxString::Format("%d", _width) + "\n";
		}
		if (_height != -1)
		{
			desc += PadTitle("Height:") + wxString::Format("%d", _height) + "\n";
		}
		if (_minwidth != -1)
		{
			desc += PadTitle("Minimum Width:") + wxString::Format("%d", _minwidth) + "\n";
		}
		if (_height != -1)
		{
			desc += PadTitle("Minimum Height:") + wxString::Format("%d", _minheight) + "\n";
		}
        if (_notes != "")
        {
            desc += "\n" + _notes + "\n";
        }

        return desc;
    }
};


void MFace::DownloadFace()
{
    if (!_faceFile.Exists())
    {
        _faceFile = MatrixFaceDownloadDialog::GetCache().GetFile(_faceLink, CACHEFOR::CACHETIME_LONG);
    }
}

class MFaceCategory
{
    void ParseCategories(wxXmlNode *n)
    {
        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
        {
            wxString nn = l->GetName().Lower().ToStdString();
            if (nn == "category")
            {
                _categories.push_back(new MVendorCategory(l, this, _vendor));
            }
        }
    }

    public:
        std::string _id;
        std::string _name;
        MFaceCategory* _parent;
        std::list<MFaceCategory*> _categories;

    std::string GetPath() const
    {
        if (_parent != nullptr)
        {
            return _parent->GetPath() + "/" + _name;
        }
        else
        {
            return _name;
        }
    }

    MFaceCategory(wxXmlNode* n, MFaceCategory* parent)
    {
        _parent = parent;
        for (wxXmlNode* e = n->GetChildren(); e != nullptr; e = e->GetNext())
        {
            wxString nn = e->GetName().Lower();
            if (nn == "id")
            {
                _id = e->GetNodeContent().ToStdString();
            }
            else if (nn == "name")
            {
                _name = e->GetNodeContent().ToStdString();
            }
            else if (nn == "categories")
            {
                ParseCategories(e);
            }
            else
            {
                wxASSERT(false);
            }
        }
    }
    virtual ~MFaceCategory()
    {
        for (auto it = _categories.begin(); it != _categories.end(); ++it)
        {
            delete *it;
        }
    }
};

class FaceBaseTreeItemData : public wxTreeItemData
{
public:
    FaceBaseTreeItemData(std::string type) : _type(type) { }

    std::string GetType() const { return _type; }

private:
    std::string _type;
};

class FCategoryTreeItemData : public FaceBaseTreeItemData
{
public:
    FCategoryTreeItemData(MFaceCategory* category) : FaceBaseTreeItemData("Category"), _category(category) { }

    MFaceCategory* GetCategory() const { return _category; }

private:
    MFaceCategory * _category;
};

class MFaceTreeItemData : public FaceBaseTreeItemData
{
public:
    MFaceTreeItemData(MFace* face) : FaceBaseTreeItemData("Face"), _face(face) { }

    MFace* GetFace() const { return _face; }

private:
    MFace * _face;
};

//(*IdInit(MatrixFaceDownloadDialog)
const long MatrixFaceDownloadDialog::ID_TREECTRL1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL3 = wxNewId();
const long MatrixFaceDownloadDialog::ID_STATICBITMAP1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_TEXTCTRL1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_STATICTEXT8 = wxNewId();
const long MatrixFaceDownloadDialog::ID_HYPERLINKCTRL4 = wxNewId();
const long MatrixFaceDownloadDialog::ID_STATICTEXT4 = wxNewId();
const long MatrixFaceDownloadDialog::ID_HYPERLINKCTRL2 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL2 = wxNewId();
const long MatrixFaceDownloadDialog::ID_BUTTON2 = wxNewId();
const long MatrixFaceDownloadDialog::ID_STATICBITMAP2 = wxNewId();
const long MatrixFaceDownloadDialog::ID_BUTTON3 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL5 = wxNewId();
const long MatrixFaceDownloadDialog::ID_TEXTCTRL2 = wxNewId();
const long MatrixFaceDownloadDialog::ID_STATICTEXT7 = wxNewId();
const long MatrixFaceDownloadDialog::ID_HYPERLINKCTRL3 = wxNewId();
const long MatrixFaceDownloadDialog::ID_BUTTON1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL4 = wxNewId();
const long MatrixFaceDownloadDialog::ID_NOTEBOOK1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MatrixFaceDownloadDialog,wxDialog)
	//(*EventTable(MatrixFaceDownloadDialog)
	//*)
END_EVENT_TABLE()

MatrixFaceDownloadDialog::MatrixFaceDownloadDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(MatrixFaceDownloadDialog)
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxSize(800,600));
	Move(wxDefaultPosition);
	SetMinSize(wxSize(800,400));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxSize(10,10));
	SplitterWindow1->SetMinimumPaneSize(10);
	SplitterWindow1->SetSashGravity(0.5);
	Panel3 = new wxPanel(SplitterWindow1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	TreeCtrl_Navigator = new wxTreeCtrl(Panel3, ID_TREECTRL1, wxDefaultPosition, wxSize(200,-1), wxTR_FULL_ROW_HIGHLIGHT|wxTR_HIDE_ROOT|wxTR_ROW_LINES|wxTR_SINGLE|wxTR_DEFAULT_STYLE|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_TREECTRL1"));
	FlexGridSizer2->Add(TreeCtrl_Navigator, 1, wxALL|wxEXPAND, 5);
	Panel3->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel3);
	FlexGridSizer2->SetSizeHints(Panel3);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	NotebookPanels = new wxNotebook(Panel1, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	PanelVendor = new wxPanel(NotebookPanels, ID_PANEL2, wxPoint(43,60), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(1);
	StaticBitmap_VendorImage = new wxStaticBitmap(PanelVendor, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(256,128), wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	StaticBitmap_VendorImage->SetMinSize(wxSize(256,128));
	FlexGridSizer4->Add(StaticBitmap_VendorImage, 1, wxALL|wxEXPAND, 5);
	TextCtrl_VendorDetails = new wxTextCtrl(PanelVendor, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer4->Add(TextCtrl_VendorDetails, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer5->AddGrowableCol(1);
	FlexGridSizer5->AddGrowableRow(0);
	StaticText6 = new wxStaticText(PanelVendor, ID_STATICTEXT8, _("Facebook:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer5->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HyperlinkCtrl_Facebook = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL4, _("http://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxNO_BORDER, _T("ID_HYPERLINKCTRL4"));
	FlexGridSizer5->Add(HyperlinkCtrl_Facebook, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(PanelVendor, ID_STATICTEXT4, _("Website:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer5->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HyperlinkCtrl_Website = new wxHyperlinkCtrl(PanelVendor, ID_HYPERLINKCTRL2, _("http://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxNO_BORDER, _T("ID_HYPERLINKCTRL2"));
	FlexGridSizer5->Add(HyperlinkCtrl_Website, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
	PanelVendor->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(PanelVendor);
	FlexGridSizer4->SetSizeHints(PanelVendor);
	Panel_Item = new wxPanel(NotebookPanels, ID_PANEL4, wxPoint(41,9), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	FlexGridSizer6->AddGrowableRow(1);
	ItemImagePanel = new wxPanel(Panel_Item, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer7->AddGrowableCol(1);
	FlexGridSizer7->AddGrowableRow(0);
	Button_Prior = new wxButton(ItemImagePanel, ID_BUTTON2, _("<"), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer7->Add(Button_Prior, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBitmap_ModelImage = new wxStaticBitmap(ItemImagePanel, ID_STATICBITMAP2, wxNullBitmap, wxDefaultPosition, wxSize(256,256), wxSIMPLE_BORDER, _T("ID_STATICBITMAP2"));
	StaticBitmap_ModelImage->SetMinSize(wxSize(256,256));
	FlexGridSizer7->Add(StaticBitmap_ModelImage, 1, wxALL|wxEXPAND, 5);
	Button_Next = new wxButton(ItemImagePanel, ID_BUTTON3, _(">"), wxDefaultPosition, wxSize(30,-1), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer7->Add(Button_Next, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ItemImagePanel->SetSizer(FlexGridSizer7);
	FlexGridSizer7->Fit(ItemImagePanel);
	FlexGridSizer7->SetSizeHints(ItemImagePanel);
	FlexGridSizer6->Add(ItemImagePanel, 1, wxALL|wxEXPAND, 0);
	TextCtrl_ModelDetails = new wxTextCtrl(Panel_Item, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer6->Add(TextCtrl_ModelDetails, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer8->AddGrowableCol(1);
	StaticText5 = new wxStaticText(Panel_Item, ID_STATICTEXT7, _("Web Link:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer8->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HyperlinkCtrl_ModelWebLink = new wxHyperlinkCtrl(Panel_Item, ID_HYPERLINKCTRL3, _("http://xlights.org"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_LEFT|wxNO_BORDER, _T("ID_HYPERLINKCTRL3"));
	FlexGridSizer8->Add(HyperlinkCtrl_ModelWebLink, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer6->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
	Button_InsertModel = new wxButton(Panel_Item, ID_BUTTON1, _("Insert Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer6->Add(Button_InsertModel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_Item->SetSizer(FlexGridSizer6);
	FlexGridSizer6->Fit(Panel_Item);
	FlexGridSizer6->SetSizeHints(Panel_Item);
	NotebookPanels->AddPage(PanelVendor, _("Vendor"), false);
	NotebookPanels->AddPage(Panel_Item, _("Item"), false);
	FlexGridSizer3->Add(NotebookPanels, 1, wxALL|wxEXPAND, 5);
	Panel1->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel1);
	FlexGridSizer3->SetSizeHints(Panel1);
	SplitterWindow1->SplitVertically(Panel3, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();

	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_ITEM_ACTIVATED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnTreeCtrl_NavigatorItemActivated);
	Connect(ID_TREECTRL1,wxEVT_COMMAND_TREE_SEL_CHANGED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnTreeCtrl_NavigatorSelectionChanged);
	Connect(ID_HYPERLINKCTRL4,wxEVT_COMMAND_HYPERLINK,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnHyperlinkCtrl_FacebookClick);
	Connect(ID_HYPERLINKCTRL2,wxEVT_COMMAND_HYPERLINK,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnHyperlinkCtrl_WebsiteClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnButton_PriorClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnButton_NextClick);
	Connect(ID_HYPERLINKCTRL3,wxEVT_COMMAND_HYPERLINK,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnHyperlinkCtrl_ModelWebLinkClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnButton_InsertModelClick);
	Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnNotebookPanelsPageChanged);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnClose);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnResize);
	//*)

    SetSize(800, 600);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("File cache size: %d", _cache.size());

    PopulateModelPanel((MModel*)nullptr);
    PopulateVendorPanel(nullptr);

    ValidateWindow();
}

bool MatrixFaceDownloadDialog::DlgInit()
{
    if (LoadTree())
    {
        ValidateWindow();
        return true;
    }

    return false;
}

wxXmlDocument* MatrixFaceDownloadDialog::GetXMLFromURL(wxURI url, std::string& filename) const
{
    filename = "";
    wxFileName fn = wxFileName(MatrixFaceDownloadDialog::GetCache().GetFile(url, CACHEFOR::CACHETIME_SESSION));
    if (fn.Exists())
    {
        filename = fn.GetFullPath();
        return new wxXmlDocument(fn.GetFullPath());
    }

    return nullptr;
}

bool MatrixFaceDownloadDialog::LoadTree()
{
    //const std::string facelink = "http://nutcracker123.com/xlights/faces/xlights_faces.xml";
    const std::string facelink = "http://localhost/xlights/faces/xlights_faces.xml";

    std::string filename;
    wxXmlDocument* vd = GetXMLFromURL(wxURI(facelink), filename);
    if (vd != nullptr && vd->IsOk())
    {
        wxXmlNode* root = vd->GetRoot();

        for (auto v = root->GetChildren(); v != nullptr; v = v->GetNext())
        {
            if (v->GetName().Lower() == "face")
            {
				MFace* f = new MFace(v);
				_faces.push_back(f);
            }
			else if (v->GetName().Lower() == "categories")
			{
				LoadCategories(v);
			}
        }
    }
    if (vd != nullptr)
    {
        delete vd;
    }

    TreeCtrl_Navigator->DeleteAllItems();
    wxTreeItemId root = TreeCtrl_Navigator->AddRoot("Faces");
	AddHierachy(root, _categories);
    for (auto it = _faces.begin(); it != _faces.end(); ++it)
    {
        wxTreeItemId v = TreeCtrl_Navigator->AppendItem(root, (*it)->_name, -1, -1, new MFaceTreeItemData(*it));
        TreeCtrl_Navigator->Expand(v);
    }

    if (_faces.size() == 0)
    {
        wxMessageBox("Unable to retrieve any faces", "Error");
        return false;
    }

    return true;
}

void MatrixFaceDownloadDialog::AddHierachy(wxTreeItemId id, std::list<MFaceCategory*> categories)
{
    for (auto it = categories.begin(); it != categories.end(); ++it)
    {
        wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(id, (*it)->_name, -1, -1, new FCategoryTreeItemData(*it));
        AddHierachy(tid, (*it)->_categories);
        TreeCtrl_Navigator->Expand(tid);
        AddFaces(tid, (*it)->_id);
    }
}

void MatrixFaceDownloadDialog::AddFaces(wxTreeItemId v, std::string categoryId)
{
    auto faces = vendor->GetFaces(categoryId);

    for (auto it = faces.begin(); it != faces.end(); ++it)
    {
		wxTreeItemId tid = TreeCtrl_Navigator->AppendItem(v, (*it)->_name, -1, -1, new MFaceTreeItemData(*it));
    }
}

MatrixFaceDownloadDialog::~MatrixFaceDownloadDialog()
{
	//(*Destroy(MatrixFaceDownloadDialog)
	//*)

    _cache.Save();

    for (auto it = _faces.begin(); it != _faces.end(); ++it)
    {
        delete *it;
    }
}

void MatrixFaceDownloadDialog::OnButton_InsertFaceClick(wxCommandEvent& event)
{
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr && ((VendorBaseTreeItemData*)tid)->GetType() == "Face")
        {
            ((MFaceTreeItemData*)tid)->GetFace()->DownloadFace();
            _faceFile = ((MFaceTreeItemData*)tid)->GetFace()->_faceFile.GetFullPath();
        }
    }

    EndDialog(wxID_OK);
}

void MatrixFaceDownloadDialog::OnTreeCtrl_NavigatorItemActivated(wxTreeEvent& event)
{
    ValidateWindow();
}

void MatrixFaceDownloadDialog::OnTreeCtrl_NavigatorSelectionChanged(wxTreeEvent& event)
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
            std::string type = ((FaceBaseTreeItemData*)tid)->GetType();

            if (type == "Face")
            {
                NotebookPanels->GetPage(0)->Show();
                NotebookPanels->SetSelection(0);
                TreeCtrl_Navigator->SetFocus();
                PopulateFacePanel(((MModelTreeItemData*)tid)->GetModel());
            }
            else
            {
                NotebookPanels->GetPage(0)->Hide();
            }
        }
        else
        {
            NotebookPanels->GetPage(0)->Hide();
        }
    }
    else
    {
        NotebookPanels->GetPage(0)->Hide();
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

void MatrixFaceDownloadDialog::ValidateWindow()
{
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr)
        {
            int imageCount = 0;
            std::string type = ((FaceBaseTreeItemData*)tid)->GetType();

            if (type == "Face")
            {
                Button_InsertFace->Enable();
            }
            else
            {
                Button_InsertFace->Disable();
            }
        }
    }
}

void MatrixFaceDownloadDialog::LoadImage(wxStaticBitmap* sb, wxImage* image) const
{
    if (image->GetWidth() == 0 || image->GetHeight() == 0) return;

    wxImage disp = image->Copy();

    float x = (float)sb->GetSize().GetWidth() / (float)disp.GetWidth();
    float y = (float)sb->GetSize().GetHeight() / (float)disp.GetHeight();
    float scale = std::min(x, y);

    sb->SetBitmap(disp.Rescale((float)disp.GetWidth() * scale, (float)disp.GetHeight() * scale));
}

void MatrixFaceDownloadDialog::LoadFaceImage(imageFile)
{
    if (imageFile->Exists())
    {
        _faceImage.LoadFile(imageFile->GetFullPath());
        if (_faceImage.IsOk())
        {
            LoadImage(StaticBitmap_FaceImage, &_faceImage);
        }
    }
}

void MatrixFaceDownloadDialog::PopulateFacePanel(MFace* face)
{
    if (face == nullptr)
    {
        StaticBitmap_FaceImage->Hide();
        TextCtrl_FaceDetails->Hide();
        NotebookPanels->GetPage(0)->Hide();
        return;
    }

    face->DownloadImages();
    if (face->_imageFile.GetPath != "")
    {
        StaticBitmap_FaceImage->Show();
        LoadFaceImage(face->_faceFile);
    }
    else
    {
        StaticBitmap_FaceImage->Hide();
    }

    TextCtrl_FaceDetails->Show();
    TextCtrl_FaceDetails->SetValue(face->GetDescription());

    if (face->_faceLink.BuildURI() == "")
    {
        Button_InsertFace->Hide();
    }
    else
    {
        Button_InsertFace->Show();
    }

    Panel_Item->Layout();
}

void MatrixFaceDownloadDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(wxID_CLOSE);
}

void MatrixFaceDownloadDialog::OnResize(wxSizeEvent& event)
{
    wxDialog::OnSize(event);

	if (StaticBitmap_FaceImage->IsShown() && _faceImage.IsOk())
	{
		if (_faceImage.GetSize() != StaticBitmap_FaceImage->GetSize())
		{
			LoadImage(StaticBitmap_FaceImage, &_faceImage);
		}
	}

    ItemImagePanel->Refresh();
}
