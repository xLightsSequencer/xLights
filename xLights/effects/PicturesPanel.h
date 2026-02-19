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

//(*Headers(PicturesPanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxFlexGridSizer;
class wxGridBagSizer;
class wxNotebook;
class wxNotebookEvent;
class wxSlider;
class wxStaticBitmap;
class wxStaticText;
class wxTextCtrl;
//*)

#include <wx/filepicker.h>
#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class SequenceMedia;

class xlPictureFilePickerCtrl : public BulkEditFilePickerCtrl {
public:
    xlPictureFilePickerCtrl(wxWindow *parent,
        wxWindowID id,
        const wxString& path = wxEmptyString,
        const wxString& message = wxFileSelectorPromptStr,
        const wxString& wildcard = wxFileSelectorDefaultWildcardStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFLP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFilePickerCtrlNameStr)
        : BulkEditFilePickerCtrl(parent, id, path, message, wxImage::GetImageExtWildcard(), pos, size, style, validator, name) {

    }
    virtual ~xlPictureFilePickerCtrl() {}
};

class PicturesPanel: public xlEffectPanel
{
	public:

		PicturesPanel(wxWindow* parent);
		virtual ~PicturesPanel();
		virtual void ValidateWindow() override;

		//(*Declarations(PicturesPanel)
		BulkEditCheckBox* CheckBox_LoopGIF;
		BulkEditCheckBox* CheckBox_Pictures_PixelOffsets;
		BulkEditCheckBox* CheckBox_Pictures_Shimmer;
		BulkEditCheckBox* CheckBox_Pictures_WrapX;
		BulkEditCheckBox* CheckBox_SuppressGIFBackground;
		BulkEditCheckBox* CheckBox_TransparentBlack;
		BulkEditChoice* Choice_Pictures_Direction;
		BulkEditChoice* Choice_Scaling;
		BulkEditSlider* Slider1;
		BulkEditSlider* Slider_PicturesEndXC;
		BulkEditSlider* Slider_PicturesEndYC;
		BulkEditSlider* Slider_PicturesXC;
		BulkEditSlider* Slider_PicturesYC;
		BulkEditSlider* Slider_Pictures_EndScale;
		BulkEditSlider* Slider_Pictures_StartScale;
		BulkEditSliderF1* Slider_Pictures_FR;
		BulkEditSliderF1* Slider_Pictures_Speed;
		BulkEditTextCtrl* TextCtrl3;
		BulkEditValueCurveButton* BitmapButton_PicturesXC;
		BulkEditValueCurveButton* BitmapButton_PicturesYC;
		wxButton* AIGenerateButton;
		wxButton* SelectButton;
		wxPanel* PictureEndPositionPanel;
		wxStaticBitmap* BitmapPreview;
		wxStaticText* StaticText160;
		wxStaticText* StaticText161;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText46;
		wxStaticText* StaticText68;
		wxStaticText* StaticText96;
		wxStaticText* StaticText_Pictures_XC;
		wxStaticText* StaticText_Pictures_YC;
		wxTextCtrl* FileNameCtrl;
		xlLockButton* BitmapButton_PicturesDirection;
		xlLockButton* BitmapButton_PicturesFrameRateAdj;
		xlLockButton* BitmapButton_PicturesSpeed;
		//*)

	protected:

		//(*Identifiers(PicturesPanel)
		static const wxWindowID ID_STATICBITMAP1;
		static const wxWindowID ID_BUTTON1;
		static const wxWindowID ID_BUTTON2;
		static const wxWindowID ID_TEXTCTRL_Pictures_Filename;
		static const wxWindowID ID_STATICTEXT_Pictures_Direction;
		static const wxWindowID ID_CHOICE_Pictures_Direction;
		static const wxWindowID ID_BITMAPBUTTON_CHOICE_Pictures_Direction;
		static const wxWindowID ID_STATICTEXT_Pictures_Speed;
		static const wxWindowID IDD_SLIDER_Pictures_Speed;
		static const wxWindowID ID_TEXTCTRL_Pictures_Speed;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Pictures_Speed;
		static const wxWindowID ID_STATICTEXT_Pictures_FrameRateAdj;
		static const wxWindowID IDD_SLIDER_Pictures_FrameRateAdj;
		static const wxWindowID ID_TEXTCTRL_Pictures_FrameRateAdj;
		static const wxWindowID ID_BITMAPBUTTON_SLIDER_Pictures_FrameRateAdj;
		static const wxWindowID ID_CHECKBOX_Pictures_PixelOffsets;
		static const wxWindowID ID_CHOICE_Scaling;
		static const wxWindowID ID_CHECKBOX_Pictures_Shimmer;
		static const wxWindowID ID_CHECKBOX_LoopGIF;
		static const wxWindowID ID_CHECKBOX_SuppressGIFBackground;
		static const wxWindowID ID_CHECKBOX_Pictures_TransparentBlack;
		static const wxWindowID IDD_SLIDER_Pictures_TransparentBlack;
		static const wxWindowID ID_TEXTCTRL_Pictures_TransparentBlack;
		static const wxWindowID ID_STATICTEXT_PicturesXC;
		static const wxWindowID ID_SLIDER_PicturesXC;
		static const wxWindowID ID_CHECKBOX_Pictures_WrapX;
		static const wxWindowID IDD_TEXTCTRL_PicturesXC;
		static const wxWindowID ID_VALUECURVE_PicturesXC;
		static const wxWindowID ID_STATICTEXT_PicturesYC;
		static const wxWindowID IDD_TEXTCTRL_PicturesYC;
		static const wxWindowID ID_VALUECURVE_PicturesYC;
		static const wxWindowID ID_SLIDER_PicturesYC;
		static const wxWindowID ID_PANEL43;
		static const wxWindowID ID_STATICTEXT_PicturesEndXC;
		static const wxWindowID ID_SLIDER_PicturesEndXC;
		static const wxWindowID IDD_TEXTCTRL_PicturesEndXC;
		static const wxWindowID ID_STATICTEXT_PicturesEndYC;
		static const wxWindowID IDD_TEXTCTRL_PicturesEndYC;
		static const wxWindowID ID_SLIDER_PicturesEndYC;
		static const wxWindowID ID_PANEL45;
		static const wxWindowID ID_STATICTEXT_Pictures_StartScale;
		static const wxWindowID ID_SLIDER_Pictures_StartScale;
		static const wxWindowID IDD_TEXTCTRL_Pictures_StartScale;
		static const wxWindowID ID_PANEL1;
		static const wxWindowID ID_STATICTEXT_Pictures_EndScale;
		static const wxWindowID ID_SLIDER_Pictures_EndScale;
		static const wxWindowID IDD_TEXTCTRL_Pictures_EndScale;
		static const wxWindowID ID_PANEL2;
		static const wxWindowID IDD_NOTEBOOK_Pictures_Positions;
		//*)

	public:

		//(*Handlers(PicturesPanel)
		void OnChoicePicturesDirectionSelect(wxCommandEvent& event);
		void OnAIGenerateButtonClick(wxCommandEvent& event);
		void OnSelectButtonClick(wxCommandEvent& event);
		//*)

		// Provide access to sequence media for the image preview thumbnail
		void SetSequenceMedia(SequenceMedia* media) { _sequenceMedia = media; }

		DECLARE_EVENT_TABLE()

private:
		void UpdatePreviewBitmap(const wxString& filename);
		SequenceMedia* _sequenceMedia = nullptr;
};
