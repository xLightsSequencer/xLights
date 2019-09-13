#ifndef CHECKBOXSELECTDIALOG_H
#define CHECKBOXSELECTDIALOG_H

//(*Headers(CheckboxSelectDialog)
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

class CheckboxSelectDialog: public wxDialog
{
    void ValidateWindow();

	public:

        CheckboxSelectDialog(wxWindow* parent, const wxString &title, const wxArrayString& items, const wxArrayString& itemsSelected = wxArrayString(), wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~CheckboxSelectDialog();
        wxArrayString GetSelectedItems() const;

		//(*Declarations(CheckboxSelectDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxCheckListBox* CheckListBox_Items;
		//*)

	protected:

		//(*Identifiers(CheckboxSelectDialog)
		static const long ID_CHECKLISTBOXITEMS;
		static const long ID_BUTTONOK;
		static const long ID_BUTTONCANCEL;
		//*)

		static const long ID_MCU_SELECTALL;
		static const long ID_MCU_SELECTNONE;
		static const long ID_MCU_SELECT_HIGH;
        static const long ID_MCU_DESELECT_HIGH;

	private:

		//(*Handlers(CheckboxSelectDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnCheckListBox_ItemsToggled(wxCommandEvent& event);
		//*)

		void OnListRClick(wxContextMenuEvent& event);
		void OnPopup(wxCommandEvent &event);

        void SelectAllLayers(bool select = true);
		void SelectHighLightedLayers(bool select = true);

		DECLARE_EVENT_TABLE()
};

#endif
