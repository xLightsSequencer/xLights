#ifndef STRANDNODENAMESDIALOG_H
#define STRANDNODENAMESDIALOG_H

//(*Headers(StrandNodeNamesDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/grid.h>
#include <wx/dialog.h>
//*)

class StrandNodeNamesDialog: public wxDialog
{
	public:

		StrandNodeNamesDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~StrandNodeNamesDialog();

		//(*Declarations(StrandNodeNamesDialog)
		wxGrid* StrandsGrid;
		wxGrid* NodesGrid;
		//*)

	protected:

		//(*Identifiers(StrandNodeNamesDialog)
		static const long ID_GRID2;
		static const long ID_GRID1;
		//*)

	private:

		//(*Handlers(StrandNodeNamesDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
