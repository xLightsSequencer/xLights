#ifndef HEADER_85D1CC366A7F7DAE
#define HEADER_85D1CC366A7F7DAE

#pragma once

class ServiceManager;

//(*Headers(AIImageDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class AIImageDialog: public wxDialog
{
    public:

        AIImageDialog(wxWindow* parent, ServiceManager* sm, const wxString& topic, const wxString& AIPrompt, wxWindowID id = wxID_ANY);
        virtual ~AIImageDialog();

        wxString GetGeneratedImagePath() const { return generatedImagePath; }

        //(*Declarations(AIImageDialog)
        wxButton* CancelButton;
        wxButton* DirButton;
        wxButton* FilenameButton;
        wxButton* GenerateButton;
        wxButton* OkButton;
        wxHtmlWindow* ResultHTMLCtrl;
        wxStaticText* StaticText1;
        wxStaticText* StaticText2;
        wxStaticText* StaticText3;
        wxStaticText* StaticText6;
        wxTextCtrl* DirectoryCtrl;
        wxTextCtrl* FilenameCtrl;
        wxTextCtrl* FreeFormText;
        wxTextCtrl* TextCtrl1;
        //*)


    protected:

        //(*Identifiers(AIImageDialog)
        static const wxWindowID ID_STATICTEXT1;
        static const wxWindowID ID_PROMPT;
        static const wxWindowID ID_STATICTEXT3;
        static const wxWindowID ID_DIRBUTTON;
        static const wxWindowID ID_DIRECTORY;
        static const wxWindowID ID_STATICTEXT6;
        static const wxWindowID ID_FNAMEBUTTON;
        static const wxWindowID ID_FILENAME;
        static const wxWindowID ID_STATICTEXT2;
        static const wxWindowID ID_AIPROMPT;
        static const wxWindowID ID_HTMLWINDOW1;
        static const wxWindowID ID_GENERATE;
        static const wxWindowID ID_OK;
        static const wxWindowID ID_CANCEL;
        //*)

    private:

        //(*Handlers(AIImageDialog)
        void OnGenerateButtonClick(wxCommandEvent& event);
        void OnOkButtonClick(wxCommandEvent& event);
        void OnCancelButtonClick(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnDirectoryButtonClick(wxCommandEvent& event);
        void OnFilenameButtonClick(wxCommandEvent& event);
        //*)

        DECLARE_EVENT_TABLE()

        ServiceManager* _serviceManager;
        wxString generatedImagePath;
};
#endif // header guard 

