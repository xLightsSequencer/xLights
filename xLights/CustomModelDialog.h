#ifndef CUSTOMMODELDIALOG_H
#define CUSTOMMODELDIALOG_H

//(*Headers(CustomModelDialog)
#include <wx/dialog.h>
class wxGrid;
class wxSpinEvent;
class wxStdDialogButtonSizer;
class wxStaticText;
class wxBitmapButton;
class wxFlexGridSizer;
class wxSpinCtrl;
class wxButton;
class wxGridEvent;
//*)

class CustomModel;

class CustomModelDialog: public wxDialog
{
	public:

		CustomModelDialog(wxWindow* parent);
		virtual ~CustomModelDialog();

		//(*Declarations(CustomModelDialog)
		wxButton* Button_CustomModelZoomIn;
		wxGrid* GridCustom;
		wxBitmapButton* BitmapButtonCustomPaste;
		wxBitmapButton* BitmapButtonCustomCopy;
		wxSpinCtrl* HeightSpin;
		wxBitmapButton* BitmapButtonCustomCut;
		wxButton* Button_CustomModelZoomOut;
		wxFlexGridSizer* Sizer1;
		wxSpinCtrl* WidthSpin;
		//*)
    
    
        void Setup(CustomModel *m);
        void Save(CustomModel *m);
        void ResizeCustomGrid();
	protected:
        void CutOrCopyToClipboard(bool isCut);

		//(*Identifiers(CustomModelDialog)
		static const long ID_SPINCTRL1;
		static const long ID_SPINCTRL2;
		static const long ID_BITMAPBUTTON_CUSTOM_CUT;
		static const long ID_BITMAPBUTTON_CUSTOM_COPY;
		static const long ID_BITMAPBUTTON_CUSTOM_PASTE;
		static const long ID_BUTTON_CustomModelZoomIn;
		static const long ID_BUTTON_CustomModelZoomOut;
		static const long ID_GRID_Custom;
		//*)

	public:

		//(*Handlers(CustomModelDialog)
		void OnWidthSpinChange(wxSpinEvent& event);
		void OnHeightSpinChange(wxSpinEvent& event);
		void OnBitmapButtonCustomCutClick(wxCommandEvent& event);
		void OnBitmapButtonCustomCopyClick(wxCommandEvent& event);
		void OnBitmapButtonCustomPasteClick(wxCommandEvent& event);
		void OnButton_CustomModelZoomInClick(wxCommandEvent& event);
		void OnButton_CustomModelZoomOutClick(wxCommandEvent& event);
		void OnButtonCustomModelHelpClick(wxCommandEvent& event);
		void OnGridCustomCellChange(wxGridEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
