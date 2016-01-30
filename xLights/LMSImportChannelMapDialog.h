#ifndef LMSIMPORTCHANNELMAPDIALOG_H
#define LMSIMPORTCHANNELMAPDIALOG_H

//(*Headers(LMSImportChannelMapDialog)
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/choice.h>
//*)

#include <map>
#include <vector>
#include "Color.h"

class SequenceElements;
class xLightsFrame;
class Model;

class LMSImportChannelMapDialog: public wxDialog
{
	public:
   
		LMSImportChannelMapDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LMSImportChannelMapDialog();
    
        void Init();

		//(*Declarations(LMSImportChannelMapDialog)
		wxFlexGridSizer* SizerMap;
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* FlexGridSizer2;
		wxSpinCtrl* TimeAdjustSpinCtrl;
		wxCheckBox* MapByStrand;
		wxFlexGridSizer* Sizer;
		wxButton* AddModelButton;
		wxChoice* ModelsChoice;
		wxPanel* TimeAdjustPanel;
		wxGrid* ChannelMapGrid;
		//*)

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
    
        std::vector<std::string> channelNames;
        std::map<std::string, xlColor> channelColors;
        std::vector<std::string> ccrNames;
        std::vector<std::string> modelNames;
	protected:

		//(*Identifiers(LMSImportChannelMapDialog)
		static const long ID_CHOICE1;
		static const long ID_BUTTON_ADDMODEL;
		static const long ID_SPINCTRL1;
		static const long ID_PANEL1;
		static const long ID_CHECKBOX1;
		static const long ID_GRID1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:
        void SetupByNode();
        void SetupByStrand();
        void AddModel(Model &cls);

		//(*Handlers(LMSImportChannelMapDialog)
		void OnMapByStrandClick(wxCommandEvent& event);
		void OnAddModelButtonClick(wxCommandEvent& event);
		void OnChannelMapGridCellChange(wxGridEvent& event);
		void OnChannelMapGridCellLeftDClick(wxGridEvent& event);
		void OnChannelMapGridEditorShown(wxGridEvent& event);
		void OnChannelMapGridEditorHidden(wxGridEvent& event);
		void LoadMapping(wxCommandEvent& event);
		void SaveMapping(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
