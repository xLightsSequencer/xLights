#ifndef MODELGROUPPANEL_H
#define MODELGROUPPANEL_H

#include <wx/dnd.h>

//(*Headers(ModelGroupPanel)
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
//*)

class ModelManager;
class LayoutPanel;

wxDECLARE_EVENT(EVT_MGDROP, wxCommandEvent);

class MGTextDropTarget : public wxTextDropTarget
{
public:
    MGTextDropTarget(wxWindow *owner, wxListCtrl* list, wxString type) { _owner = owner; _list = list; _type = type; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    wxWindow *_owner;
    wxListCtrl* _list;
    wxString _type;
};

class ModelGroupPanel: public wxPanel
{
	public:

		ModelGroupPanel(wxWindow* parent,ModelManager &Models,LayoutPanel *xl,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelGroupPanel();

		void UpdatePanel(const std::string group);
		void AddPreviewChoice(const std::string name);

		//(*Declarations(ModelGroupPanel)
		wxBitmapButton* ButtonAddModel;
		wxBitmapButton* ButtonRemoveModel;
		wxListCtrl* ListBoxAddToModelGroup;
		wxBitmapButton* ButtonMoveDown;
		wxStaticText* StaticText2;
		wxBitmapButton* ButtonMoveUp;
		wxChoice* ChoiceModelLayoutType;
		wxChoice* ChoicePreviews;
		wxStaticText* LabelModelGroupName;
		wxListCtrl* ListBoxModelsInGroup;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxFlexGridSizer* Panel_Sizer;
		wxStaticText* GridSizeLabel;
		wxSpinCtrl* SizeSpinCtrl;
		wxStaticText* StaticText5;
		//*)

	protected:

		//(*Identifiers(ModelGroupPanel)
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT6;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_CHOICE_PREVIEWS;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT2;
		static const long ID_LISTCTRL1;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL2;
		//*)

	private:
        LayoutPanel *layoutPanel;
        ModelManager &mModels;
        std::string mGroup;
        void SaveGroupChanges();

		//(*Handlers(ModelGroupPanel)
		void OnButtonAddToModelGroupClick(wxCommandEvent& event);
		void OnButtonRemoveFromModelGroupClick(wxCommandEvent& event);
		void OnButtonUpClick(wxCommandEvent& event);
		void OnButtonDownClick(wxCommandEvent& event);
		void OnChoiceModelLayoutTypeSelect(wxCommandEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnPaint1(wxPaintEvent& event);
		void OnChoicePreviewsSelect(wxCommandEvent& event);
		void OnSizeSpinCtrlChange(wxSpinEvent& event);
		void OnListBoxAddToModelGroupBeginDrag(wxListEvent& event);
		void OnListBoxAddToModelGroupItemSelect(wxListEvent& event);
		void OnListBoxModelsInGroupBeginDrag(wxListEvent& event);
		void OnListBoxModelsInGroupItemSelect(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        void OnDrop(wxCommandEvent& event);
        bool IsItemSelected(wxListCtrl* ctrl, int item);
        void ValidateWindow();
        int GetSelectedModelCount();

        bool _dragRowModel;
        bool _dragRowNonModel;
        void AddSelectedModels(int index);
        void RemoveSelectedModels();
        void MoveSelectedModelsTo(int indexTo);
        void ClearSelections(wxListCtrl* listCtrl, long stateMask);
};

#endif
