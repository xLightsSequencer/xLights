#ifndef GENERATECUSTOMMODELDIALOG_H
#define GENERATECUSTOMMODELDIALOG_H

#include "VideoReader.h"
#include "xlights_out.h"

//(*Headers(GenerateCustomModelDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/aui/aui.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/statbmp.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#ifdef __WXOSX__
#include "osxMacUtils.h"
#endif

#include <wx/xml/xml.h>
#include <wx/progdlg.h>

class GenerateCustomModelDialog: public wxDialog
{
    wxXmlDocument* _network;
    wxDateTime _starttime;
    bool _continue;

    void ValidateWindow();
    void SetBulbs(bool nodes, int count, int startch, int node, int ms, int intensity, xOutput* xout);
    bool InitialiseOutputs(xOutput* xOut);
    void UpdateProgress(wxProgressDialog& pd, int totaltime);
    int FindStartFrame(VideoReader* vr);
    float CalcFrameBrightness(AVFrame* image);

#ifdef __WXOSX__
    AppNapSuspender _sleepData;
    void EnableSleepModes()
    {
        _sleepData.resume();
    }
    void DisableSleepModes()
    {
        _sleepData.suspend();
    }
#else
    void EnableSleepModes() {}
    void DisableSleepModes() {}
#endif

    public:
		GenerateCustomModelDialog(wxWindow* parent, wxXmlDocument* network, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~GenerateCustomModelDialog();

		//(*Declarations(GenerateCustomModelDialog)
		wxStaticBitmap* StaticBitmap1;
		wxButton* Button_GCM_Generate;
		wxSlider* Slider_Intensity;
		wxButton* Button_PCM_Run;
		wxSpinCtrl* SpinCtrl_NC_Count;
		wxFileDialog* FileDialog1;
		wxButton* Button_Continue;
		wxTextCtrl* TextCtrl_Message;
		wxTextCtrl* TextCtrl_GCM_Filename;
		wxPanel* Panel_Generate;
		wxAuiNotebook* AuiNotebook1;
		wxRadioBox* RadioBox1;
		wxPanel* Panel_Prepare;
		wxSpinCtrl* SpinCtrl_StartChannel;
		//*)

	protected:

		//(*Identifiers(GenerateCustomModelDialog)
		static const long ID_RADIOBOX1;
		static const long ID_SPINCTRL_NC_Count;
		static const long ID_SPINCTRL_StartChannel;
		static const long ID_SLIDER_Intensity;
		static const long ID_BUTTON_PCM_Run;
		static const long ID_PANEL_Prepare;
		static const long ID_TEXTCTRL_GCM_Filename;
		static const long ID_BUTTON_GCM_SelectFile;
		static const long ID_BUTTON_GCM_Generate;
		static const long ID_STATICBITMAP1;
		static const long ID_TEXTCTRL_Message;
		static const long ID_BUTTON_Continue;
		static const long ID_PANEL_Generate;
		static const long ID_AUINOTEBOOK1;
		//*)

	private:

		//(*Handlers(GenerateCustomModelDialog)
		void OnButton_GCM_SelectFileClick(wxCommandEvent& event);
		void OnTextCtrl_GCM_FilenameText(wxCommandEvent& event);
		void OnButton_GCM_GenerateClick(wxCommandEvent& event);
		void OnButton_PCM_RunClick(wxCommandEvent& event);
		void OnButton_ContinueClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
