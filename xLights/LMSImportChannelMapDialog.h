#ifndef LMSIMPORTCHANNELMAPDIALOG_H
#define LMSIMPORTCHANNELMAPDIALOG_H

//(*Headers(LMSImportChannelMapDialog)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <map>
#include "Color.h"

class SequenceElements;
class xLightsFrame;
class ModelClass;

class LMSImportChannelMapDialog: public wxDialog
{
	public:
   
		LMSImportChannelMapDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LMSImportChannelMapDialog();
    
        void Init();

		//(*Declarations(LMSImportChannelMapDialog)
		wxGrid* ChannelMapGrid;
		wxChoice* ModelsChoice;
		wxFlexGridSizer* Sizer;
		wxCheckBox* MapByStrand;
		wxButton* AddModelButton;
		//*)

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
        wxArrayString channelNames;
        std::map<wxString, xlColor> channelColors;
        wxArrayString ccrNames;
        wxArrayString modelNames;
	protected:

		//(*Identifiers(LMSImportChannelMapDialog)
		static const long ID_CHOICE1;
		static const long ID_BUTTON_ADDMODEL;
		static const long ID_CHECKBOX1;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:
        void SetupByNode();
        void SetupByStrand();
        void AddModel(ModelClass &cls);

		//(*Handlers(LMSImportChannelMapDialog)
		void OnMapByStrandClick(wxCommandEvent& event);
		void OnAddModelButtonClick(wxCommandEvent& event);
		void OnChannelMapGridCellChange(wxGridEvent& event);
		void OnChannelMapGridCellLeftDClick(wxGridEvent& event);
		void OnChannelMapGridEditorShown(wxGridEvent& event);
		void OnChannelMapGridEditorHidden(wxGridEvent& event);
		void LoadMapping(wxCommandEvent& event);
		void SaveMapping(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
