#ifndef MODELGROUPPANEL_H
#define MODELGROUPPANEL_H

//(*Headers(ModelGroupPanel)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
//*)

class ModelManager;
class xLightsFrame;

class ModelGroupPanel: public wxPanel
{
	public:

		ModelGroupPanel(wxWindow* parent,ModelManager &Models,xLightsFrame *xl,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelGroupPanel();

		void UpdatePanel(const std::string group);

		//(*Declarations(ModelGroupPanel)
		wxStaticText* StaticText2;
		wxChoice* ChoiceModelLayoutType;
		wxButton* Button_SaveGroupChanges;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxStaticText* GridSizeLabel;
		wxSpinCtrl* SizeSpinCtrl;
		wxListBox* ListBoxModelsInGroup;
		wxListBox* ListBoxAddToModelGroup;
		//*)

	protected:

		//(*Identifiers(ModelGroupPanel)
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_SPINCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_LISTBOX_ADD_TO_MODEL_GROUP;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_MODELS_IN_GROUP;
		static const long ID_BUTTON1;
		//*)

	private:
        xLightsFrame *xlights;
        ModelManager &mModels;
        std::string mGroup;

		//(*Handlers(ModelGroupPanel)
		void OnButtonAddToModelGroupClick(wxCommandEvent& event);
		void OnButtonRemoveFromModelGroupClick(wxCommandEvent& event);
		void OnButtonUpClick(wxCommandEvent& event);
		void OnButtonDownClick(wxCommandEvent& event);
		void OnChoiceModelLayoutTypeSelect(wxCommandEvent& event);
		void OnPaint(wxPaintEvent& event);
		void OnPaint1(wxPaintEvent& event);
		void OnButton_SaveGroupChangesClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
