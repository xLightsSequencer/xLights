#ifndef MHFEATUREDIALOG_H
#define MHFEATUREDIALOG_H

//(*Headers(MhFeatureDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/sizer.h>
//*)

class MhFeature;

class MhFeatureDialog: public wxDialog
{
	public:

		MhFeatureDialog(std::vector<std::unique_ptr<MhFeature>>& _features, wxXmlNode* _node_xml, wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MhFeatureDialog();

		//(*Declarations(MhFeatureDialog)
		wxButton* Button_AddFeature;
		wxButton* Button_DeleteFeature;
		wxFlexGridSizer* FlexGridSizer2;
		wxFlexGridSizer* FlexGridSizerMain;
		wxGrid* Grid1;
		//*)

	protected:

		//(*Identifiers(MhFeatureDialog)
		static const long ID_GRID1;
		static const long ID_BUTTON_AddFeature;
		static const long ID_BUTTON_DeleteFeature;
		//*)

	private:

		//(*Handlers(MhFeatureDialog)
		void OnButton_AddFeatureClick(wxCommandEvent& event);
		void OnButton_DeleteFeatureClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()

        wxXmlNode* node_xml;
        std::vector<std::unique_ptr<MhFeature>>& features;
};

#endif
