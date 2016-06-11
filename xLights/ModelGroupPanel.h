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
//*)

class ModelManager;
class xLightsFrame;

class ModelGroupPanel: public wxPanel
{
	public:

		ModelGroupPanel(wxWindow* parent,ModelManager &Models,xLightsFrame *xl,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelGroupPanel();

		void UpdatePanel(const std::string group);
		void AddPreviewChoice(const std::string name);

		//(*Declarations(ModelGroupPanel)
		wxStaticText* StaticText2;
		wxChoice* ChoiceModelLayoutType;
		wxChoice* ChoicePreviews;
		wxStaticText* LabelModelGroupName;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxFlexGridSizer* Panel_Sizer;
		wxStaticText* GridSizeLabel;
		wxSpinCtrl* SizeSpinCtrl;
		wxStaticText* StaticText5;
		wxListBox* ListBoxModelsInGroup;
		wxListBox* ListBoxAddToModelGroup;
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
		static const long ID_LISTBOX_ADD_TO_MODEL_GROUP;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_STATICTEXT1;
		static const long ID_LISTBOX_MODELS_IN_GROUP;
		//*)

	private:
        xLightsFrame *xlights;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
