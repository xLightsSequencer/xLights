#ifndef LMSIMPORTCHANNELMAPDIALOG_H
#define LMSIMPORTCHANNELMAPDIALOG_H

//(*Headers(LMSImportChannelMapDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <map>
#include <vector>
#include "Color.h"

#include <wx/filename.h>

class SequenceElements;
class xLightsFrame;
class Model;

class LMSImportChannelMapDialog: public wxDialog
{
    static wxColourData _colorData;
    wxFileName _filename;
        bool _dirty;
    public:
   
		LMSImportChannelMapDialog(wxWindow* parent, const wxFileName &filename, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LMSImportChannelMapDialog();
    
        void Init(bool allModels = false);

		//(*Declarations(LMSImportChannelMapDialog)
		wxSpinCtrl* TimeAdjustSpinCtrl;
		wxButton* Button_Ok;
		wxGrid* ChannelMapGrid;
		wxChoice* ModelsChoice;
		wxPanel* TimeAdjustPanel;
		wxFlexGridSizer* Sizer;
		wxFlexGridSizer* FlexGridSizer2;
		wxButton* Button_Cancel;
		wxCheckBox* MapByStrand;
		wxFlexGridSizer* FlexGridSizer1;
		wxFlexGridSizer* SizerMap;
		wxButton* AddModelButton;
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
        void OnButton_OkClick(wxCommandEvent& event);
        void OnButton_CancelClick(wxCommandEvent& event);
        //*)

		DECLARE_EVENT_TABLE()
};

#endif
