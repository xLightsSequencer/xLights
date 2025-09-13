#ifndef AICOLORPALETTEDIALOG_H
#define AICOLORPALETTEDIALOG_H

//(*Headers(AIColorPaletteDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
//*)

class AIColorPaletteDialog: public wxDialog
{
    public:

        AIColorPaletteDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
        virtual ~AIColorPaletteDialog();

        //(*Declarations(AIColorPaletteDialog)
        wxButton* CancelButton;
        wxButton* GenerateButton;
        wxButton* OkButon;
        wxHtmlWindow* ResultHTMLCtrl;
        wxRadioButton* FreeFormRadioButton;
        wxRadioButton* SongRadioButton;
        wxTextCtrl* FreeFormText;
        wxTextCtrl* SongTextCtrl;
        //*)

    
        wxArrayString GetColorStrings();
    protected:

        //(*Identifiers(AIColorPaletteDialog)
        static const wxWindowID ID_RADIOBUTTON1;
        static const wxWindowID ID_TEXTCTRL1;
        static const wxWindowID ID_RADIOBUTTON2;
        static const wxWindowID ID_TEXTCTRL2;
        static const wxWindowID ID_HTMLWINDOW1;
        static const wxWindowID ID_BUTTON1;
        static const wxWindowID ID_OK;
        static const wxWindowID ID_CANCEL;
        //*)

    private:

        //(*Handlers(AIColorPaletteDialog)
        void OnSongRadioButtonSelect(wxCommandEvent& event);
        void OnSongTextCtrlTextEnter(wxCommandEvent& event);
        void OnFreeFormRadioButtonSelect(wxCommandEvent& event);
        void OnGenerateButtonClick(wxCommandEvent& event);
        void OnOkButonClick(wxCommandEvent& event);
        void OnCancelButtonClick(wxCommandEvent& event);
        void OnSongTextCtrlText(wxCommandEvent& event);
        //*)

        DECLARE_EVENT_TABLE()
    
    
        wxArrayString colors;
    
    void createFreeFormFromSong();
};

#endif
