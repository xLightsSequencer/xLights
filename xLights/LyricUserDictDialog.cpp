#include "LyricUserDictDialog.h"

//(*InternalHeaders(LyricUserDictDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "PhonemeDictionary.h"
#include "xLightsApp.h"
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <log4cpp/Category.hh>

//(*IdInit(LyricUserDictDialog)
const long LyricUserDictDialog::ID_TEXTCTRL_NEW_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_STATICTEXT1 = wxNewId();
const long LyricUserDictDialog::ID_COMBOBOX_OLD_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_ADD_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_GRID_USER_LYRIC_DICT = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_DELETE_ROW = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_LYRIC_OK = wxNewId();
const long LyricUserDictDialog::ID_BUTTON_LYRIC_CANCEL = wxNewId();
//*)

BEGIN_EVENT_TABLE(LyricUserDictDialog,wxDialog)
	//(*EventTable(LyricUserDictDialog)
	//*)
END_EVENT_TABLE()

LyricUserDictDialog::LyricUserDictDialog(PhonemeDictionary* dictionary, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    m_dictionary = dictionary;

	//(*Initialize(LyricUserDictDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxGridSizer* GridSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;

	Create(parent, wxID_ANY, _("Edit User Lyric Dictionary"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	GridSizer1 = new wxGridSizer(3, 1, 0, 0);
	FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer1->AddGrowableRow(2);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Label"));
	TextCtrlNewLyric = new wxTextCtrl(this, ID_TEXTCTRL_NEW_LYRIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_NEW_LYRIC"));
	StaticBoxSizer1->Add(TextCtrlNewLyric, 2, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Copy From"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	StaticBoxSizer1->Add(StaticText1, 0, wxALL|wxFIXED_MINSIZE, 5);
	ComboBoxOldLyric = new wxComboBox(this, ID_COMBOBOX_OLD_LYRIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_COMBOBOX_OLD_LYRIC"));
	StaticBoxSizer1->Add(ComboBoxOldLyric, 2, wxALL|wxEXPAND, 5);
	ButtonAddLyric = new wxButton(this, ID_BUTTON_ADD_LYRIC, _("Add"), wxDefaultPosition, wxSize(75,24), 0, wxDefaultValidator, _T("ID_BUTTON_ADD_LYRIC"));
	StaticBoxSizer1->Add(ButtonAddLyric, 1, wxALL|wxSHAPED|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizer1, 0, wxALL|wxFIXED_MINSIZE, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Label"));
	GridUserLyricDict = new wxGrid(this, ID_GRID_USER_LYRIC_DICT, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxHSCROLL, _T("ID_GRID_USER_LYRIC_DICT"));
	GridUserLyricDict->CreateGrid(0,2);
	GridUserLyricDict->SetMinSize(wxSize(-1,100));
	GridUserLyricDict->EnableEditing(true);
	GridUserLyricDict->EnableGridLines(true);
	GridUserLyricDict->SetDefaultColSize(100, true);
	GridUserLyricDict->SetColLabelValue(0, _("Word"));
	GridUserLyricDict->SetColLabelValue(1, _("Phonemes"));
	GridUserLyricDict->SetDefaultCellFont( GridUserLyricDict->GetFont() );
	GridUserLyricDict->SetDefaultCellTextColour( GridUserLyricDict->GetForegroundColour() );
	StaticBoxSizer2->Add(GridUserLyricDict, 1, wxALL|wxEXPAND, 5);
	ButtonDeleteRow = new wxButton(this, ID_BUTTON_DELETE_ROW, _("Delete Row"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_DELETE_ROW"));
	StaticBoxSizer2->Add(ButtonDeleteRow, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizer2, 3, wxALL|wxEXPAND|wxSHAPED, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonLyricOK = new wxButton(this, ID_BUTTON_LYRIC_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LYRIC_OK"));
	BoxSizer1->Add(ButtonLyricOK, 1, wxALL, 5);
	ButtonLyricCancel = new wxButton(this, ID_BUTTON_LYRIC_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LYRIC_CANCEL"));
	BoxSizer1->Add(ButtonLyricCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 0, wxALL|wxFIXED_MINSIZE, 5);
	GridSizer1->Add(FlexGridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridSizer1);
	GridSizer1->Fit(this);
	GridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_ADD_LYRIC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonAddLyricClick);
	Connect(ID_BUTTON_DELETE_ROW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonDeleteRowClick);
	Connect(ID_BUTTON_LYRIC_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonLyricOKClick);
	Connect(ID_BUTTON_LYRIC_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonLyricCancelClick);
	//*)

    ComboBoxOldLyric->Set(m_dictionary->GetPhonemeList());
    ReadUserDictionary();
}

LyricUserDictDialog::~LyricUserDictDialog()
{
	//(*Destroy(LyricUserDictDialog)
	//*)
}


void LyricUserDictDialog::OnButtonAddLyricClick(wxCommandEvent& event)
{
    if (TextCtrlNewLyric->GetValue().IsEmpty()) return;

    if (m_dictionary->ContainsPhoneme(TextCtrlNewLyric->GetValue())
        || DoesGridContain(TextCtrlNewLyric->GetValue()))
    {
        wxMessageBox("Word Already Exists In Phoneme Dictionary");
        return;
    }

    InsertRow(TextCtrlNewLyric->GetValue(), m_dictionary->GetPhoneme(ComboBoxOldLyric->GetValue()));
    TextCtrlNewLyric->Clear();
    ComboBoxOldLyric->Clear();
    //m_dictionary->GetPhoneme();
}

void LyricUserDictDialog::OnButtonDeleteRowClick(wxCommandEvent& event)
{
    wxArrayInt indexs = GridUserLyricDict->GetSelectedRows();
    wxString msg = "Delete Slect Phonemes";
    int answer = wxMessageBox(msg,
        "Delete Phoneme",
        wxYES_NO, this);
    // no: return
    if (answer == wxNO) {
        return;
    }

    for (int x = 0; x < indexs.size(); x++)
    {
        int i = indexs[x];
        GridUserLyricDict->DeleteRows(i);
    }
}

void LyricUserDictDialog::OnButtonLyricOKClick(wxCommandEvent& event)
{
    //check for valid Phonemes..
    for (int i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
    {
        if (GridUserLyricDict->GetCellValue(i, 1).IsEmpty() || !IsValidPhoneme(GridUserLyricDict->GetCellValue(i, 1)))
        {
            wxString msg = "Invalid Phonemes for: " + GridUserLyricDict->GetCellValue(i, 0);
            wxMessageBox(msg,"Invalid Phonemes");
            return;
        }
    }
    //save and insert
    WriteUserDictionary();
    EndDialog(wxID_OK);
}

void LyricUserDictDialog::OnButtonLyricCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void LyricUserDictDialog::ReadUserDictionary()
{
    wxString filename = "user_dictionary";
    wxFileName phonemeFile = wxFileName::DirName(xLightsApp::showDir);
    phonemeFile.SetFullName(filename);
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath());
        phonemeFile.SetFullName(filename);
    }
    if (!wxFile::Exists(phonemeFile.GetFullPath())) {
        phonemeFile = wxFileName(wxStandardPaths::Get().GetResourcesDir(), filename);
    }
    if (!wxFile::Exists(phonemeFile.GetFullPath()))
    {
        return;
    }

    wxFileInputStream input(phonemeFile.GetFullPath());
    wxTextInputStream text(input);

    while (input.IsOk() && !input.Eof())
    {
        wxString line = text.ReadLine();
        line = line.Trim();
        if (line.Left(1) == "#" || line.Length() == 0)
            continue; // skip comments

        wxArrayString strList = wxSplit(line, ' ');
        InsertRow(strList[0], strList);
    }
}

void LyricUserDictDialog::WriteUserDictionary()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString filename = "user_dictionary";
    wxFileName phonemeFile = wxFileName::DirName(xLightsApp::showDir);
    phonemeFile.SetFullName(filename);

    wxFile f(phonemeFile.GetFullPath());
    if (!f.Create(filename, true) || !f.IsOpened())
    {
        logger_base.error("Could Not Save user_dictionary");
        return;
    }
    for (int i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
    {
        wxArrayString strList = wxSplit(GridUserLyricDict->GetCellValue(i, 1), ' ');
        strList.Insert("", 0);
        strList.Insert(GridUserLyricDict->GetCellValue(i, 0), 0);
        f.Write(wxJoin(strList,' '));
        f.Write('\n');
        m_dictionary->InsertPhoneme(strList);
    }

    f.Close();
}

void LyricUserDictDialog::InsertRow(const wxString & text, wxArrayString phonemeList)
{
    if (phonemeList.size() > 2)
    {
        phonemeList.RemoveAt(0, 2);//phonemeList has a name and a space at the beginning

        int row = GridUserLyricDict->GetNumberRows();
        GridUserLyricDict->InsertRows(row);
        GridUserLyricDict->SetCellValue(row, 0, text);
        GridUserLyricDict->SetReadOnly(row, 1);
        GridUserLyricDict->SetCellValue(row, 1, wxJoin(phonemeList, ' '));
    }
}

bool LyricUserDictDialog::DoesGridContain(const wxString & text)
{
    for(int i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
    {
        if (GridUserLyricDict->GetCellValue(i, 0) == text)
            return true;
    }
    return false;
}

bool LyricUserDictDialog::IsValidPhoneme(const wxString & text)
{
    wxArrayString strList = wxSplit(text, ' ');
    for (int i = 0; i < strList.size(); i++)
    {
        if (!m_dictionary->ContainsPhonemeMap(strList[i]))
            return false;
    }
    return false;
}
