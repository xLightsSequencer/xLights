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

//(*Headers(OtherSettingsPanel)
#include <wx/panel.h>
//*)

class wxCheckBox;
class wxChoice;
class wxCommandEvent;
class wxSpinCtrlDouble;
class wxSpinDoubleEvent;
class wxStaticText;
class wxTextCtrl;
class xLightsFrame;

class OtherSettingsPanel: public wxPanel
{
	public:

		OtherSettingsPanel(wxWindow* parent,xLightsFrame *f, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~OtherSettingsPanel();

		//(*Declarations(OtherSettingsPanel)
		//*)
		// Hand-added (outside the wxSmith guards): preview graphics backend
		// selector, only present on builds with the Vulkan backend compiled in.
		wxChoice* GraphicsBackendChoice = nullptr;

        virtual bool TransferDataFromWindow() override;
        virtual bool TransferDataToWindow() override;

	private:
        xLightsFrame *frame;

        wxCheckBox* CheckBox_BatchRenderPromptIssues = nullptr;
        wxCheckBox* CheckBox_EnablePositionZones = nullptr;
        wxCheckBox* CheckBox_IgnoreVendorModelRecommendations = nullptr;
        wxCheckBox* CheckBox_PurgeDownloadCache = nullptr;
        wxCheckBox* CheckBox_RecycleTips = nullptr;
        wxCheckBox* CheckBox_ShowZoneIndicator = nullptr;
        wxCheckBox* CheckBox_UseCustomColorPicker = nullptr;
        wxCheckBox* ExcludeAudioCheckBox = nullptr;
        wxCheckBox* ExcludeVideosCheckBox = nullptr;
        wxCheckBox* GPURenderCheckbox = nullptr;
        wxCheckBox* ShaderCheckbox = nullptr;
        wxChoice* Choice_AliasPromptBehavior = nullptr;
        wxChoice* Choice_LinkControllerUpload = nullptr;
        wxChoice* Choice_MinTipLevel = nullptr;
        wxSpinCtrlDouble* CtrlPingInterval = nullptr;
        wxTextCtrl* eMailTextControl = nullptr;

        // Description labels for controls that may be hidden per-platform, so
        // the description can be hidden alongside its control.
        wxWindow* GPURenderHint = nullptr;
        wxWindow* ShaderHint = nullptr;
        wxWindow* IgnoreVendorHint = nullptr;

        // Write changes back immediately on platforms where the preferences
        // editor applies as-you-go.
        void ApplyIfImmediate();

		//(*Handlers(OtherSettingsPanel)
		//*)
        void OnControlChanged(wxCommandEvent& event);
        void OnSpinCtrlDoubleChange(wxSpinDoubleEvent& event);
};
