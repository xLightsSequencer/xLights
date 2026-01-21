#ifndef HEADER_85D1CC366A7F7DAE
#define HEADER_85D1CC366A7F7DAE

#pragma once

class ServiceManager;

//(*Headers(AIImageDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "ai/aiBase.h"

class AIImageDialog: public wxDialog
{
    public:

        AIImageDialog(wxWindow* parent, aiBase* service, wxWindowID id = wxID_ANY);
        virtual ~AIImageDialog();


        //(*Declarations(AIImageDialog)
        wxButton* CloseButton;
        wxButton* GenerateButton;
        wxButton* SaveButton;
        wxFlexGridSizer* MainSizer;
        wxFlexGridSizer* ParametersSizer;
        wxFlexGridSizer* ResultBoxSizer;
        wxStaticBitmap* ResultBitmap;
        wxStaticBoxSizer* ParametersBox;
        wxStaticBoxSizer* ResultsBox;
        wxStaticText* ErrorText;
        wxStaticText* StaticText1;
        wxTextCtrl* PromptBox;
        //*)


    protected:

        //(*Identifiers(AIImageDialog)
        static const wxWindowID ID_STATICTEXT1;
        static const wxWindowID ID_PROMPT;
        static const wxWindowID ID_STATICBITMAP1;
        static const wxWindowID ID_STATICTEXT2;
        static const wxWindowID ID_GENERATE;
        static const wxWindowID ID_BUTTON1;
        static const wxWindowID ID_OK;
        //*)

    private:

        //(*Handlers(AIImageDialog)
        void OnGenerateButtonClick(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnSaveButtonClicked(wxCommandEvent& event);
        void OnCloseButtonClick(wxCommandEvent& event);
        //*)

        DECLARE_EVENT_TABLE()

        aiBase::AIImageGenerator *generator = nullptr;
};
#endif // header guard 

