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


#include <vector>
#include <string>

class SequenceViewManager;

class ModelViewSelector: public wxDialog
{
	public:

		ModelViewSelector(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelViewSelector();
        std::vector<std::string> ModelsToAdd;
		std::vector<std::string> TimingsToAdd;

		void SetSequenceElementsModelsViews(SequenceElements* elements,wxXmlNode* models, wxXmlNode *modelGroups, SequenceViewManager* sequenceViewManager, int which_view = MASTER_VIEW);
        void Initialize();

		//(*Declarations(ModelViewSelector)
		wxStaticText* StaticTextType;
		wxButton* ButtonClose;
		wxStaticText* StaticText2;
		wxListBox* ListBoxElements;
		wxStaticText* StaticText1;
		wxButton* ButtonAdd;
		wxListBox* ListBoxTimings;
		//*)

	protected:

		//(*Identifiers(ModelViewSelector)
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_TIMINGS;
		static const long ID_STATICTEXT_TYPE;
		static const long ID_LISTBOX_ELEMENTS;
		static const long ID_STATICTEXT1;
		static const long ID_BUTTON_ADD;
		static const long ID_BUTTON_CLOSE;
		//*)

	private:
        SequenceElements* mSequenceElements;
        wxXmlNode* mModels;
        SequenceViewManager* _sequenceViewManager;
        wxXmlNode *mModelGroups;
        int mWhichView;

        void PopulateModelsToAdd();

		//(*Handlers(ModelViewSelector)
		void OnButtonAddClick(wxCommandEvent& event);
		void OnButtonCloseClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
