/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LyricUserDictDialog.h"

//(*InternalHeaders(LyricUserDictDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "PhonemeDictionary.h"
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>
#include <log4cpp/Category.hh>
#include "UtilFunctions.h"

//(*IdInit(LyricUserDictDialog)
const long LyricUserDictDialog::ID_TEXTCTRL_NEW_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_STATICTEXT1 = wxNewId();
const long LyricUserDictDialog::ID_TEXTCTRL_OLD_LYRIC = wxNewId();
const long LyricUserDictDialog::ID_TEXTCTRL_PHEMONES = wxNewId();
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

void LyricUserDictDialog::ValidateWindow()
{
    if (GridUserLyricDict->GetSelectedRows().size() == 0)
    {
        ButtonDeleteRow->Disable();
    }
    else
    {
        ButtonDeleteRow->Enable();
    }
    if (TextCtrlNewLyric->GetValue() == "")
    {
        ButtonAddLyric->Disable();
    }
    else
    {
        ButtonAddLyric->Enable();
    }

    bool allValid = true;
    for (auto i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
    {
        if (GridUserLyricDict->GetCellValue(i, 1).IsEmpty() || !IsValidPhoneme(GridUserLyricDict->GetCellValue(i, 1).Upper()))
        {
            allValid = false;
            break;
        }
    }

    ButtonLyricOK->Enable(allValid);
}

LyricUserDictDialog::LyricUserDictDialog(PhonemeDictionary* dictionary, const wxString &showDirectory, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    m_dictionary = dictionary;
    m_showDirectory = showDirectory;

	//(*Initialize(LyricUserDictDialog)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;

	Create(parent, wxID_ANY, _("Edit User Lyric Dictionary"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(3, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Add Word"));
	TextCtrlNewLyric = new wxTextCtrl(this, ID_TEXTCTRL_NEW_LYRIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_NEW_LYRIC"));
	StaticBoxSizer1->Add(TextCtrlNewLyric, 1, wxALL|wxEXPAND, 5);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Copy From"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	StaticBoxSizer1->Add(StaticText1, 0, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	TextCtrlOldLyric = new wxTextCtrl(this, ID_TEXTCTRL_OLD_LYRIC, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_OLD_LYRIC"));
	StaticBoxSizer1->Add(TextCtrlOldLyric, 1, wxALL|wxEXPAND, 5);
	TextCtrlPhonems = new wxTextCtrl(this, ID_TEXTCTRL_PHEMONES, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_PHEMONES"));
	StaticBoxSizer1->Add(TextCtrlPhonems, 1, wxALL|wxEXPAND, 5);
	ButtonAddLyric = new wxButton(this, ID_BUTTON_ADD_LYRIC, _("Add"), wxDefaultPosition, wxSize(75,24), 0, wxDefaultValidator, _T("ID_BUTTON_ADD_LYRIC"));
	StaticBoxSizer1->Add(ButtonAddLyric, 0, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Edit Word"));
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
	FlexGridSizer1->Add(StaticBoxSizer2, 3, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ButtonLyricOK = new wxButton(this, ID_BUTTON_LYRIC_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LYRIC_OK"));
	BoxSizer1->Add(ButtonLyricOK, 1, wxALL, 5);
	ButtonLyricCancel = new wxButton(this, ID_BUTTON_LYRIC_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_LYRIC_CANCEL"));
	BoxSizer1->Add(ButtonLyricCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_RIGHT|wxFIXED_MINSIZE, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL_NEW_LYRIC,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&LyricUserDictDialog::OnTextCtrlNewLyricText);
	Connect(ID_TEXTCTRL_OLD_LYRIC,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&LyricUserDictDialog::OnTextCtrlOldLyricText);
	Connect(ID_BUTTON_ADD_LYRIC,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonAddLyricClick);
	Connect(ID_GRID_USER_LYRIC_DICT,wxEVT_GRID_LABEL_LEFT_CLICK,(wxObjectEventFunction)&LyricUserDictDialog::OnGridUserLyricDictLabelLeftClick);
	Connect(ID_GRID_USER_LYRIC_DICT,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&LyricUserDictDialog::OnGridUserLyricDictCellChanged);
	Connect(ID_GRID_USER_LYRIC_DICT,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&LyricUserDictDialog::OnGridUserLyricDictCellSelect);
	Connect(ID_BUTTON_DELETE_ROW,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonDeleteRowClick);
	Connect(ID_BUTTON_LYRIC_OK,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonLyricOKClick);
	Connect(ID_BUTTON_LYRIC_CANCEL,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LyricUserDictDialog::OnButtonLyricCancelClick);
	//*)

    GridUserLyricDict->HideRowLabels();
    GridUserLyricDict->SetSelectionMode(wxGrid::wxGridSelectRows);

    SetEscapeId(ButtonLyricCancel->GetId());

    TextCtrlOldLyric->AutoComplete(m_dictionary->GetPhonemeList());
    ReadUserDictionary();
    ValidateWindow();
}

LyricUserDictDialog::~LyricUserDictDialog()
{
	//(*Destroy(LyricUserDictDialog)
	//*)
}


void LyricUserDictDialog::OnButtonAddLyricClick(wxCommandEvent& event)
{
    if (TextCtrlNewLyric->GetValue().IsEmpty()) return;

    //check if word is "marked" to be removed from main dictionary
    const bool found = (std::find(m_removeList.begin(), m_removeList.end(), TextCtrlNewLyric->GetValue().Upper()) != m_removeList.end());

    //only error if word is in main dictionary and not marked to be deleted on close
    // or its already populated in the dialog
    if ((m_dictionary->ContainsPhoneme(TextCtrlNewLyric->GetValue().Upper()) && !found)
        || DoesGridContain(TextCtrlNewLyric->GetValue().Upper()))
    {
        DisplayError("Word '" + TextCtrlNewLyric->GetValue() +"' Already Exists In Phoneme Dictionary", this);
        return;
    }

    InsertRow(TextCtrlNewLyric->GetValue().Upper(), m_dictionary->GetPhoneme(TextCtrlOldLyric->GetValue().Upper()));
    GridUserLyricDict->SelectRow(GridUserLyricDict->GetNumberRows() - 1);
    GridUserLyricDict->GoToCell(GridUserLyricDict->GetNumberRows() - 1, 1);
    TextCtrlNewLyric->Clear();
    TextCtrlOldLyric->Clear();
    TextCtrlPhonems->Clear();
    ValidateWindow();
}

void LyricUserDictDialog::OnButtonDeleteRowClick(wxCommandEvent& event)
{
    wxArrayInt indexs = GridUserLyricDict->GetSelectedRows();
    const wxString& msg = "Delete Select Phonemes";
    const int answer = wxMessageBox(msg, "Delete Phonemes", wxYES_NO, this);

    if (answer == wxNO)
    {
        return;
    }

    for (auto x = 0; x < indexs.size(); x++)
    {
        auto i = indexs[x];
        wxString name = GridUserLyricDict->GetCellValue(i, 0);

        //"marked" removeds word so they can be removed from the main dictionary on dialog ok event
        const bool found = (std::find(m_removeList.begin(), m_removeList.end(), name) != m_removeList.end());
        if(!found)
            m_removeList.Add(name);

        GridUserLyricDict->DeleteRows(i);
    }
    ValidateWindow();
}

void LyricUserDictDialog::OnButtonLyricOKClick(wxCommandEvent& event)
{
    //check for valid Phonemes..
    for (auto i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
    {
        if (GridUserLyricDict->GetCellValue(i, 1).IsEmpty() || !IsValidPhoneme(GridUserLyricDict->GetCellValue(i, 1).Upper()))
        {
            const auto msg = "Invalid Phonemes for: " + GridUserLyricDict->GetCellValue(i, 0);
            DisplayError(msg, this);
            return;
        }
    }
    //Remove Old Phoneme
    for (const auto& str : m_removeList)
    {
        m_dictionary->RemovePhoneme(str);
    }
    //save and insert
    WriteUserDictionary();
    EndDialog(wxID_OK);
}

void LyricUserDictDialog::OnButtonLyricCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void LyricUserDictDialog::ReadUserDictionary() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    const wxString filename = "user_dictionary";
    wxFileName phonemeFile = wxFileName::DirName(m_showDirectory);
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
        logger_base.error("Could Not Find user_dictionary file");
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

void LyricUserDictDialog::WriteUserDictionary() const
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    const wxString filename = "user_dictionary";
    wxFileName phonemeFile = wxFileName::DirName(m_showDirectory);
    phonemeFile.SetFullName(filename);

    wxFile f(phonemeFile.GetFullPath(), wxFile::write);

    if (f.IsOpened())
    {
        for (auto i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
        {
            wxArrayString str_list = wxSplit(GridUserLyricDict->GetCellValue(i, 1).Upper(), ' ');
            str_list.Insert("", 0);
            str_list.Insert(GridUserLyricDict->GetCellValue(i, 0).Upper(), 0);
            f.Write(wxJoin(str_list, ' '));
            f.Write('\n');
            m_dictionary->InsertPhoneme(str_list);
        }

        f.Close();
    }
    else
    {
        logger_base.error("Could Not Save user_dictionary file");
    }
}

void LyricUserDictDialog::InsertRow(const wxString & text, wxArrayString phonemeList) const
{
    const int row = GridUserLyricDict->GetNumberRows();
    GridUserLyricDict->InsertRows(row);
    GridUserLyricDict->SetCellValue(row, 0, text);
    GridUserLyricDict->SetReadOnly(row, 0);

    if (phonemeList.size() > 2)
    {
        phonemeList.RemoveAt(0, 2);//phonemeList has a name and a space at the beginning
        GridUserLyricDict->SetCellValue(row, 1, wxJoin(phonemeList, ' '));
    }
    else
    {
        GridUserLyricDict->SetCellValue(row, 1, "");
    }
}

bool LyricUserDictDialog::DoesGridContain(const wxString & text) const
{
    for(auto i = 0; i < GridUserLyricDict->GetNumberRows(); i++)
    {
        if (GridUserLyricDict->GetCellValue(i, 0) == text)
            return true;
    }
    return false;
}

bool LyricUserDictDialog::IsValidPhoneme(const wxString & text) const
{
    const auto& str_list = wxSplit(text, ' ');

    for (const auto& str : str_list)
    {
        if (!m_dictionary->ContainsPhonemeMap(str))
            return false;
    }
    return true;
}

void LyricUserDictDialog::OnTextCtrlOldLyricText(wxCommandEvent& event)
{
    const auto& words = wxSplit(TextCtrlOldLyric->GetValue().Upper(), ' ');
    wxArrayString phenoms;
    //loop through for multiple words
    for (const auto& word : words)
    {
        if ((m_dictionary->ContainsPhoneme(word)))
        {
            wxArrayString word_phenoms = m_dictionary->GetPhoneme(word);
            if (word_phenoms.size() > 2)
            {
                word_phenoms.RemoveAt(0, 2);//phonemeList has a name and a space at the beginning
                phenoms.insert(phenoms.end(),word_phenoms.begin(),word_phenoms.end());
            }
        }
    }

    if (!phenoms.IsEmpty())
    {
         TextCtrlPhonems->SetValue(wxJoin(phenoms, ' '));
    }
    else
    {
        TextCtrlPhonems->Clear();
    }
    ValidateWindow();
}

void LyricUserDictDialog::OnGridUserLyricDictCellSelect(wxGridEvent& event)
{
    GridUserLyricDict->SelectRow(event.GetRow());
    ValidateWindow();
}

void LyricUserDictDialog::OnGridUserLyricDictCellChanged(wxGridEvent& event)
{
    ValidateWindow();
}

void LyricUserDictDialog::OnTextCtrlNewLyricText(wxCommandEvent& event)
{
    ValidateWindow();
}

void LyricUserDictDialog::OnGridUserLyricDictLabelLeftClick(wxGridEvent& event)
{
    ValidateWindow();
}
