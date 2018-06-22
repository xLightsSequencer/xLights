#include "MatrixFaceDownloadDialog.h"

//(*InternalHeaders(MatrixFaceDownloadDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>
#include <wx/stopwatch.h>
#include "CachedFileDownloader.h"
#include <wx/log.h>
#include <log4cpp/Category.hh>

CachedFileDownloader& MatrixFaceDownloadDialog::GetCache() {
    return CachedFileDownloader::GetDefaultCache();
}

class MFace
{
public:
    wxURI _image;
    wxFileName _imageFile;
    std::string _name;
    std::string _description;
    std::string _artist;
    std::string _copyright;
    std::string _notes;
    wxURI _faceLink;
    wxFileName _faceFile;
    std::string _id;
    std::list<std::string> _categoryIds;
    int _width;
    int _height;
    int _minwidth;
    int _minheight;

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

    bool IsMinimumSizeOK(int width, int height) const
    {
        if (_minheight <= 0 && _minwidth <= 0) return true;
        if (width <= 0 && height <= 0) return true;

        bool ok = true;
        if (ok && width > 0 && _minwidth > 0)
        {
            ok = width >= _minwidth;
        }

        if (ok && height > 0 && _minheight > 0)
        {
            ok = height >= _minheight;
        }

        return ok;
    }

    MFace(wxXmlNode* n)
    {
		_width = -1;
		_height = -1;
		_minwidth = -1;
		_minheight = -1;

        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
        {
            if (l->GetType() != wxXML_COMMENT_NODE)
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
        if (_description != "")
        {
            desc += PadTitle("Description:") + _description + "\n\n";
        }
        if (_artist != "")
        {
            desc += PadTitle("Artist:") + _artist + "\n\n";
        }
        if (_copyright != "")
        {
            desc += PadTitle("Copyright:") + _copyright + "\n\n";
        }
		if (_width > 0)
		{
			desc += PadTitle("Width:") + wxString::Format("%d", _width) + "\n";
		}
		if (_height > 0)
		{
			desc += PadTitle("Height:") + wxString::Format("%d", _height) + "\n";
		}
		if (_minwidth > 0)
		{
			desc += PadTitle("Minimum Width:") + wxString::Format("%d", _minwidth) + "\n";
		}
		if (_minheight > 0)
		{
			desc += PadTitle("Minimum Height:") + wxString::Format("%d", _minheight) + "\n";
		}
        if (_notes != "")
        {
            desc += "\n" + _notes + "\n";
        }

        return desc;
    }

    void DownloadFace()
    {
        if (!_faceFile.Exists())
        {
            _faceFile = MatrixFaceDownloadDialog::GetCache().GetFile(_faceLink, CACHEFOR::CACHETIME_LONG);
        }
    }
};

class MFaceCategory
{
    void ParseCategories(wxXmlNode *n)
    {
        for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
        {
            wxString nn = l->GetName().Lower().ToStdString();
            if (nn == "category")
            {
                _categories.push_back(new MFaceCategory(l, this));
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
const long MatrixFaceDownloadDialog::ID_CHECKBOX1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL3 = wxNewId();
const long MatrixFaceDownloadDialog::ID_STATICBITMAP1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_TEXTCTRL1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_BUTTON1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_PANEL1 = wxNewId();
const long MatrixFaceDownloadDialog::ID_SPLITTERWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(MatrixFaceDownloadDialog,wxDialog)
	//(*EventTable(MatrixFaceDownloadDialog)
	//*)
END_EVENT_TABLE()

MatrixFaceDownloadDialog::MatrixFaceDownloadDialog(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _width = -1;
    _height = -1;

	//(*Initialize(MatrixFaceDownloadDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxSize(800,600));
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
	CheckBox_FilterUnsuitable = new wxCheckBox(Panel3, ID_CHECKBOX1, _("Filter Unsuitable Faces"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_FilterUnsuitable->SetValue(true);
	FlexGridSizer2->Add(CheckBox_FilterUnsuitable, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel3->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel3);
	FlexGridSizer2->SetSizeHints(Panel3);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	FlexGridSizer3->AddGrowableRow(0);
	StaticBitmap_FaceImage = new wxStaticBitmap(Panel1, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(256,128), wxSIMPLE_BORDER, _T("ID_STATICBITMAP1"));
	StaticBitmap_FaceImage->SetMinSize(wxSize(256,128));
	FlexGridSizer3->Add(StaticBitmap_FaceImage, 1, wxALL|wxEXPAND, 5);
	TextCtrl_FaceDetails = new wxTextCtrl(Panel1, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(-1,170), wxTE_MULTILINE|wxTE_READONLY|wxTE_LEFT, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer3->Add(TextCtrl_FaceDetails, 1, wxALL|wxEXPAND, 5);
	Button_InsertFace = new wxButton(Panel1, ID_BUTTON1, _("Insert Face"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_InsertFace, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnCheckBox_FilterUnsuitableClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnButton_InsertFaceClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnClose);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&MatrixFaceDownloadDialog::OnResize);
	//*)

    SetSize(800, 600);

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("File cache size: %d", GetCache().size());

    PopulateFacePanel((MFace*)nullptr);

    ValidateWindow();
}

bool MatrixFaceDownloadDialog::DlgInit(int width, int height)
{
    _width = width;
    _height = height;

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
    TreeCtrl_Navigator->DeleteAllItems();
    while (_faces.size() > 0)
    {
        delete _faces.front();
        _faces.pop_front();
    }

    while (_categories.size() > 0)
    {
        delete _categories.front();
        _categories.pop_front();
    }

    const std::string facelink = "http://nutcracker123.com/xlights/faces/xlights_faces.xml";
    //const std::string facelink = "http://localhost:3000/xlights/faces/xlights_faces.xml";

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
                if (!CheckBox_FilterUnsuitable->IsChecked() || f->IsMinimumSizeOK(_width, _height))
                {
                    _faces.push_back(f);
                }
                else
                {
                    delete f;
                }
            }
			else if (v->GetName().Lower() == "categories")
			{
                for (wxXmlNode* l = v->GetChildren(); l != nullptr; l = l->GetNext())
                {
                    wxString nn = l->GetName().Lower().ToStdString();
                    if (nn == "category")
                    {
                        _categories.push_back(new MFaceCategory(l, nullptr));
                    }
                }
            }
        }
    }
    if (vd != nullptr)
    {
        delete vd;
    }

    wxTreeItemId root = TreeCtrl_Navigator->AddRoot("Faces");
	AddHierachy(root, _categories);

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
    auto faces = GetFaces(categoryId);

    for (auto it = faces.begin(); it != faces.end(); ++it)
    {
		TreeCtrl_Navigator->AppendItem(v, (*it)->_name, -1, -1, new MFaceTreeItemData(*it));
    }
}

MatrixFaceDownloadDialog::~MatrixFaceDownloadDialog()
{
	//(*Destroy(MatrixFaceDownloadDialog)
	//*)

    GetCache().Save();

    while (_faces.size() > 0)
    {
        delete _faces.front();
        _faces.pop_front();
    }

    while (_categories.size() > 0)
    {
        delete _categories.front();
        _categories.pop_front();
    }
}

void MatrixFaceDownloadDialog::OnButton_InsertFaceClick(wxCommandEvent& event)
{
    if (TreeCtrl_Navigator->GetSelection().IsOk())
    {
        wxTreeItemData* tid = TreeCtrl_Navigator->GetItemData(TreeCtrl_Navigator->GetSelection());

        if (tid != nullptr && ((FaceBaseTreeItemData*)tid)->GetType() == "Face")
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
                TreeCtrl_Navigator->SetFocus();
                PopulateFacePanel(((MFaceTreeItemData*)tid)->GetFace());
            }
            else
            {
                PopulateFacePanel(nullptr);
            }
        }
        else
        {
            PopulateFacePanel(nullptr);
        }
    }
    else
    {
        PopulateFacePanel(nullptr);
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
    else
    {
        Button_InsertFace->Disable();
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

std::list<MFace*> MatrixFaceDownloadDialog::GetFaces(const std::string& categoryId)
{
    std::list<MFace*> res;

    for (auto it = _faces.begin(); it != _faces.end(); ++it)
    {
        if ((*it)->InCategory(categoryId))
        {
            res.push_back(*it);
        }
    }

    return res;
}

void MatrixFaceDownloadDialog::LoadFaceImage(wxFileName imageFile)
{
    if (imageFile.Exists())
    {
        _faceImage.LoadFile(imageFile.GetFullPath());
        if (_faceImage.IsOk())
        {
            LoadImage(StaticBitmap_FaceImage, &_faceImage);
        }
    }
}

void MatrixFaceDownloadDialog::PopulateFacePanel(MFace* face)
{
    wxLogNull logNo; //kludge: avoid log dialog
    if (face == nullptr)
    {
        StaticBitmap_FaceImage->Hide();
        TextCtrl_FaceDetails->Hide();
        return;
    }

    face->DownloadImages();
    if (face->_imageFile.Exists())
    {
        StaticBitmap_FaceImage->Show();
        LoadFaceImage(face->_imageFile);
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

    Panel1->Layout();
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

    Panel1->Refresh();
}

void MatrixFaceDownloadDialog::OnCheckBox_FilterUnsuitableClick(wxCommandEvent& event)
{
    LoadTree();
    ValidateWindow();
}
