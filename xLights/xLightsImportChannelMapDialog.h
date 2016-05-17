#ifndef XLIGHTSIMPORTCHANNELMAPDIALOG_H
#define XLIGHTSIMPORTCHANNELMAPDIALOG_H

//(*Headers(xLightsImportChannelMapDialog)
#include <wx/sizer.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <map>
#include <vector>
#include "Color.h"
#include <wx/arrstr.h>

class SequenceElements;
class xLightsFrame;
class Model;

class xLightsImportChannelMapDialog: public wxDialog
{
	public:
   
		xLightsImportChannelMapDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~xLightsImportChannelMapDialog();
    
        bool _dirty;
        bool Init(bool allModels = false);

		//(*Declarations(xLightsImportChannelMapDialog)
		wxButton* Button_Ok;
		wxGrid* ChannelMapGrid;
		wxChoice* ModelsChoice;
		wxFlexGridSizer* Sizer;
		wxFlexGridSizer* FlexGridSizer2;
		wxButton* Button_Cancel;
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* SizerMap;
		wxButton* AddModelButton;
		//*)

        SequenceElements *mSequenceElements;
        xLightsFrame * xlights;
    
        std::vector<std::string> channelNames;
        std::vector<std::string> ccrNames;
        std::vector<std::string> modelNames;
	protected:

		//(*Identifiers(xLightsImportChannelMapDialog)
		static const long ID_CHOICE1;
		static const long ID_BUTTON_ADDMODEL;
		static const long ID_GRID1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:
        bool allowAddModels;
        void SetupByNode();
        void SetupByStrand();
        void AddModel(Model &cls);
        wxString FindTab(wxString &line);

		//(*Handlers(xLightsImportChannelMapDialog)
		void OnMapByStrandClick(wxCommandEvent& event);
		void OnAddModelButtonClick(wxCommandEvent& event);
		void OnChannelMapGridCellChange(wxGridEvent& event);
		void OnChannelMapGridCellLeftDClick(wxGridEvent& event);
		void OnChannelMapGridEditorShown(wxGridEvent& event);
		void OnChannelMapGridEditorHidden(wxGridEvent& event);
		void LoadMapping(wxCommandEvent& event);
		void SaveMapping(wxCommandEvent& event);
		void OnResize(wxSizeEvent& event);
		void OnChannelMapGridCellLeftClick(wxGridEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
