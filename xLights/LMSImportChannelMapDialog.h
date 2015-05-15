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
		//*)

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
        wxArrayString channelNames;
        xlColorVector channelColors;
        wxArrayString ccrNames;
        wxArrayString modelNames;
	protected:

		//(*Identifiers(LMSImportChannelMapDialog)
		static const long ID_CHOICE1;
		static const long ID_BUTTON_ADDMODEL;
		static const long ID_CHECKBOX1;
		static const long ID_GRID1;
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
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
