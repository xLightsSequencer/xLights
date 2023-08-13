#ifndef FPPUPLOADPROGRESSDIALOG_H
#define FPPUPLOADPROGRESSDIALOG_H

//(*Headers(FPPUploadProgressDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class wxGauge;

class FPPUploadProgressDialog: public wxDialog
{
    public:

        FPPUploadProgressDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
        virtual ~FPPUploadProgressDialog();

        //(*Declarations(FPPUploadProgressDialog)
        wxButton* CancelButton;
        wxFlexGridSizer* scrolledWindowSizer;
        wxScrolledWindow* scrolledWindow;
        wxStaticText* ActionLabel;
        //*)

        bool isCancelled() const {return cancelled;}
        void setActionLabel(const std::string &action);
        wxGauge *addGauge(const std::string &name);
    protected:

        //(*Identifiers(FPPUploadProgressDialog)
        static const long ID_STATICTEXT1;
        static const long ID_SCROLLEDWINDOW1;
        static const long ID_BUTTON1;
        //*)

    private:
    bool cancelled = false;
    
        //(*Handlers(FPPUploadProgressDialog)
        void OnCancelButtonClick(wxCommandEvent& event);
        //*)

        DECLARE_EVENT_TABLE()
};

#endif
