#ifndef PLAYLISTDIALOG_H
#define PLAYLISTDIALOG_H

//(*Headers(PlayListDialog)
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class PlayList;
class wxXmlNode;

class PlayListDialog: public wxDialog
{
    PlayList* _playlist;
    wxXmlNode* _savedState;
    bool _dragging;
    void ValidateWindow();
    void PopulateTree();
    int GetPos(const wxTreeItemId& item);
    void HighlightDropItem(wxTreeItemId* id);
    void DeleteSelectedItem();

    public:

		PlayListDialog(wxWindow* parent, PlayList* playlist, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListDialog();
        void UpdateTree();
        void OnTreeDragEnd(wxMouseEvent& event);
        void OnTreeDragQuit(wxMouseEvent& event);
        void OnTreeMouseMove(wxMouseEvent& event);

		//(*Declarations(PlayListDialog)
		wxButton* Button_Ok;
		wxNotebook* Notebook1;
		wxTreeCtrl* TreeCtrl_PlayList;
		wxStaticText* StaticText2;
		wxPanel* Panel1;
		wxButton* Button_Cancel;
		wxPanel* Panel2;
		wxSplitterWindow* SplitterWindow1;
		//*)

	protected:

		//(*Identifiers(PlayListDialog)
		static const long ID_TREECTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL1;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

        static const long ID_MNU_ADDSTEP;
        static const long ID_MNU_ADDESEQ;
        static const long ID_MNU_ADDFSEQ;
        static const long ID_MNU_ADDALLOFF;
        static const long ID_MNU_ADDPROCESS;
        static const long ID_MNU_ADDVIDEO;
        static const long ID_MNU_ADDIMAGE;
        static const long ID_MNU_ADDDELAY;
        static const long ID_MNU_DELETE;

        bool IsPlayList(wxTreeItemId id);
        bool IsPlayListStep(wxTreeItemId id);
        void OnTreeCtrlMenu(wxCommandEvent &event);

	private:

		//(*Handlers(PlayListDialog)
		void OnTextCtrl_PlayListNameText(wxCommandEvent& event);
		void OnTreeCtrl_PlayListSelectionChanged(wxTreeEvent& event);
		void OnTreeCtrl_PlayListBeginDrag(wxTreeEvent& event);
		void OnTreeCtrl_PlayListEndDrag(wxTreeEvent& event);
		void OnTreeCtrl_PlayListItemMenu(wxTreeEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnTreeCtrl_PlayListKeyDown(wxTreeEvent& event);
		void OnNotebook1PageChanged(wxNotebookEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
