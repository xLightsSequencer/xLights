#ifndef MODELFACEDIALOG_H
#define MODELFACEDIALOG_H

//(*Headers(ModelFaceDialog)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/panel.h>
#include <wx/grid.h>
#include <wx/dialog.h>
#include <wx/choicebk.h>
//*)

#include <map>

class ModelClass;

class ModelFaceDialog: public wxDialog
{
	public:

		ModelFaceDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelFaceDialog();

		//(*Declarations(ModelFaceDialog)
		wxStaticText* StaticText1;
		wxPanel* Matrix;
		wxChoicebook* FaceTypeChoice;
		wxGrid* GridCoroFaces;
		//*)

        void SetFaceInfo(ModelClass *cls, std::map<wxString, wxString> &info);
        void GetFaceInfo(std::map<wxString, wxString> &info);
	protected:

		//(*Identifiers(ModelFaceDialog)
		static const long ID_PANEL1;
		static const long ID_GRID_COROFACES;
		static const long ID_PANEL2;
		static const long ID_STATICTEXT1;
		static const long ID_PANEL3;
		static const long ID_CHOICEBOOK1;
		//*)

	private:
        ModelClass *modelClass;

		//(*Handlers(ModelFaceDialog)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
