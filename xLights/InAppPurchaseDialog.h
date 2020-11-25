#ifndef INAPPPURCHASEDIALOG_H
#define INAPPPURCHASEDIALOG_H

//(*Headers(InAppPurchaseDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/gbsizer.h>
#include <wx/hyperlink.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>
//*)

#include <array>
#include <string>

class InAppPurchaseDialog: public wxDialog
{
	public:

		InAppPurchaseDialog(wxWindow* parent);
		virtual ~InAppPurchaseDialog();

		//(*Declarations(InAppPurchaseDialog)
		wxFlexGridSizer* FlexGridSizer1;
		wxHyperlinkCtrl* HyperlinkCtrl1;
		wxStaticText* LargeLabel;
		wxStaticText* LargeMonthlyLabel;
		wxStaticText* MediumLabel;
		wxStaticText* MediumMonthlyLabel;
		wxStaticText* SmallLabel;
		wxStaticText* SmallMonthlyLabel;
		wxStaticText* StaticText2;
		wxToggleButton* LargeButton;
		wxToggleButton* LargeMonthlyButton;
		wxToggleButton* MediumButton;
		wxToggleButton* MediumMonthlyButton;
		wxToggleButton* SmallButton;
		wxToggleButton* SmallMonthlyButton;
		//*)

        std::array<std::string, 6> prices;
        void setPrices();
    
        int getSelection();
	protected:

		//(*Identifiers(InAppPurchaseDialog)
		static const long ID_STATICTEXT4;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_TOGGLEBUTTON1;
		static const long ID_TOGGLEBUTTON2;
		static const long ID_TOGGLEBUTTON3;
		static const long ID_TOGGLEBUTTON4;
		static const long ID_TOGGLEBUTTON5;
		static const long ID_TOGGLEBUTTON6;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT7;
		static const long ID_HYPERLINKCTRL1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(InAppPurchaseDialog)
		void OnSmallButtonToggle(wxCommandEvent& event);
		void OnMediumButtonToggle(wxCommandEvent& event);
		void OnLargeButtonToggle(wxCommandEvent& event);
		void OnSmallMonthlyButtonToggle(wxCommandEvent& event);
		void OnMediumMonthlyButtonToggle(wxCommandEvent& event);
		void OnLargeMonthlyButtonToggle(wxCommandEvent& event);
		void OnCancelButtonClick(wxCommandEvent& event);
		void OnPurchaseButtonClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
