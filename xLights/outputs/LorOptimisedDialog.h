#ifndef LOROPTIMISEDDIALOG_H
#define LOROPTIMISEDDIALOG_H

//(*Headers(LorOptimisedDialog)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class LOROptimisedOutput;
class OutputManager;
class LorControllers;

class LorOptimisedDialog: public wxDialog
{
    LorControllers* _lorControllers;
    LOROptimisedOutput* _original;
    LOROptimisedOutput** _serial;
    OutputManager* _outputManager;
    void ValidateWindow();
    void LoadList();
    void EditSelected();

public:

    LorOptimisedDialog(wxWindow* parent, LOROptimisedOutput** serial, LorControllers* lorControllers, OutputManager* outputManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~LorOptimisedDialog();

    //(*Declarations(LorOptimisedDialog)
    wxButton* Button_Ok;
    wxTextCtrl* TextCtrl_Description;
    wxStaticText* StaticText2;
    wxStaticText* StaticTextRate;
    wxChoice* ChoicePort;
    wxButton* Button_Delete;
    wxStaticText* StaticTextExplanation;
    wxButton* Button_Edit;
    wxButton* Button_Add;
    wxChoice* ChoiceBaudRate;
    wxListView* ListView_Controllers;
    wxStaticText* StaticTextPort;
    //*)

protected:

    //(*Identifiers(LorOptimisedDialog)
    static const long ID_STATICTEXT_EXPLANATION;
    static const long ID_STATICTEXT_PORT;
    static const long ID_CHOICE_PORT;
    static const long ID_STATICTEXT_RATE;
    static const long ID_CHOICE_BAUD_RATE;
    static const long ID_STATICTEXT2;
    static const long ID_TEXTCTRL_DESCRIPTION;
    static const long ID_LISTVIEW_CONTROLLERS;
    static const long ID_BUTTON_ADD;
    static const long ID_BUTTON_EDIT;
    static const long ID_BUTTON_DELETE;
    static const long ID_BUTTON_OK;
    //*)

private:

    //(*Handlers(LorOptimisedDialog)
    void OnTextCtrl_DescriptionText(wxCommandEvent& event);
    void OnButton_OkClick(wxCommandEvent& event);
    void OnChoicePortSelect(wxCommandEvent& event);
    void OnChoiceBaudRateSelect(wxCommandEvent& event);
    void OnButton_DeleteClick(wxCommandEvent& event);
    void OnButton_EditClick(wxCommandEvent& event);
    void OnButton_AddClick(wxCommandEvent& event);
    void OnListView_ControllersItemSelect(wxListEvent& event);
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
