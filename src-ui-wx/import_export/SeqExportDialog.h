#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(SeqExportDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <string>

class SeqExportDialog: public wxDialog
{
    std::string _model;
    std::string _filename;
    void ValidateWindow();
    wxString GetWildcardForFormat() const;
    wxString GetDefaultName() const;
    bool PromptForFilename();

public:

    SeqExportDialog(wxWindow* parent, const std::string& model, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SeqExportDialog();
    void ModelExportTypes(bool isgroup);
    void SetExportType(bool selectedEffects, bool render);

    // The format string the user selected (e.g. "GIF Image, *.gif").
    wxString GetExportFormat() const;
    // The fully-resolved output path chosen via the Save As dialog. Empty until
    // the dialog returns wxID_OK.
    const std::string& GetExportFilename() const { return _filename; }

    //(*Declarations(SeqExportDialog)
    wxButton* ButtonCancel;
    wxButton* ButtonOk;
    wxListBox* ListBoxFormat;
    wxStaticText* StaticText1;
    //*)

protected:

    //(*Identifiers(SeqExportDialog)
    static const long ID_STATICTEXT1;
    static const long ID_LISTBOX1;
    static const long ID_BUTTON2;
    static const long ID_BUTTON3;
    //*)

private:

    //(*Handlers(SeqExportDialog)
    void OnListBoxFormatSelect(wxCommandEvent& event);
    void OnListBoxFormatDClick(wxCommandEvent& event);
    void OnButtonOkClick(wxCommandEvent& event);
    void OnButtonCancelClick(wxCommandEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};
