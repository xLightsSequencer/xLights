#ifndef LMSIMPORTCHANNELMAPDIALOG_H
#define LMSIMPORTCHANNELMAPDIALOG_H

//(*Headers(LMSImportChannelMapDialog)
#include <wx/sizer.h>
#include <wx/checkbox.h>
#include <wx/grid.h>
#include <wx/dialog.h>
//*)


class LMSImportChannelMapDialog: public wxDialog
{
	public:
   
		LMSImportChannelMapDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~LMSImportChannelMapDialog();

		//(*Declarations(LMSImportChannelMapDialog)
		wxGrid* ChannelMapGrid;
		wxFlexGridSizer* Sizer;
		wxCheckBox* MapByStrand;
		//*)

    
        void SetNames(const wxArrayString *cn,
                      const wxArrayString *sn,
                      const wxArrayString *nn,
                      const wxArrayString *ccrn);
	protected:

		//(*Identifiers(LMSImportChannelMapDialog)
		static const long ID_CHECKBOX1;
		static const long ID_GRID1;
		//*)

	private:
        void SetupByNode();
        void SetupByStrand();
    
        const wxArrayString *channelNames = nullptr;
        const wxArrayString *strandNames = nullptr;
        const wxArrayString *nodeNames = nullptr;
        const wxArrayString *ccrNames = nullptr;

		//(*Handlers(LMSImportChannelMapDialog)
		void OnMapByStrandClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
