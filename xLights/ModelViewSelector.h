#ifndef MODELVIEWSELECTOR_H
#define MODELVIEWSELECTOR_H

#include "sequencer/SequenceElements.h"

#include "wx/xml/xml.h"
//(*Headers(ModelViewSelector)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class ModelViewSelector: public wxDialog
{
	public:

		ModelViewSelector(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelViewSelector();
		wxArrayString ElementsToAdd;


		void SetSequenceElementsModelsViews(SequenceElements* elements,wxXmlNode* models, wxXmlNode *modelGroups, wxXmlNode* views);
        void Initialize(wxString type);

		//(*Declarations(ModelViewSelector)
		wxStaticText* StaticTextType;
		wxButton* ButtonClose;
		wxListBox* ListBoxElements;
		wxStaticText* StaticText1;
		wxButton* ButtonAdd;
		//*)

	protected:

		//(*Identifiers(ModelViewSelector)
		static const long ID_STATICTEXT_TYPE;
		static const long ID_LISTBOX_ELEMENTS;
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON_ADD;
		static const long ID_BUTTON_CLOSE;
		//*)

	private:
       SequenceElements* mSequenceElements;
       wxXmlNode* mModels;
       wxXmlNode* mViews;
       wxXmlNode *mModelGroups;

        void PopulateViewsToAdd();
        void PopulateModelsToAdd();

		//(*Handlers(ModelViewSelector)
		void OnButtonAddClick(wxCommandEvent& event);
		void OnButtonCloseClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
