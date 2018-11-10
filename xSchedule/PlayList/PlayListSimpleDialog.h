#ifndef PLAYLISTSIMPLEDIALOG_H
#define PLAYLISTSIMPLEDIALOG_H

//(*Headers(PlayListSimpleDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filedlg.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/treectrl.h>
//*)

class OutputManager;
class PlayList;
class wxXmlNode;
class PlayListStep;
class PlayListItem;

class PlayListSimpleDialog: public wxDialog
{
    OutputManager* _outputManager;
    PlayList* _playlist;
    PlayList* _savedState;
    bool _dragging;
    void ValidateWindow();
    void PopulateTree(PlayList* playlist, PlayListStep* step);
    int GetPos(const wxTreeItemId& item);
    void HighlightDropItem(wxTreeItemId* id);
    void DeleteSelectedItem();
    void SwapPage(wxNotebookPage* newpage, const std::string& text = "");
    wxTreeItemId FindStepTreeItem(PlayListStep* step);
    void AddItem(PlayList* playlist, PlayListStep* step, PlayListItem* newitem);
    void Clone();

    public:

		PlayListSimpleDialog(wxWindow* parent, OutputManager* outputManager, PlayList* playlist, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~PlayListSimpleDialog();
        void UpdateTree();
        void OnTreeDragEnd(wxMouseEvent& event);
        void OnTreeDragQuit(wxMouseEvent& event);
        void OnTreeMouseMove(wxMouseEvent& event);

		//(*Declarations(PlayListSimpleDialog)
		wxButton* Button_AddAudio;
		wxButton* Button_AddFSEQ;
		wxButton* Button_Cancel;
		wxButton* Button_Clone;
		wxButton* Button_Delete;
		wxButton* Button_FSEQVideo;
		wxButton* Button_Ok;
		wxFileDialog* FileDialog1;
		wxNotebook* Notebook1;
		wxPanel* Panel1;
		wxPanel* Panel2;
		wxSplitterWindow* SplitterWindow1;
		wxTreeCtrl* TreeCtrl_PlayList;
		//*)

	protected:

		//(*Identifiers(PlayListSimpleDialog)
		static const long ID_TREECTRL1;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_BUTTON7;
		static const long ID_BUTTON6;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL1;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

        bool IsPlayList(wxTreeItemId id);
        bool IsPlayListStep(wxTreeItemId id);
        void OnDropFiles(wxDropFilesEvent& event);

	private:

		//(*Handlers(PlayListSimpleDialog)
		void OnTextCtrl_PlayListNameText(wxCommandEvent& event);
		void OnTreeCtrl_PlayListSelectionChanged(wxTreeEvent& event);
		void OnTreeCtrl_PlayListBeginDrag(wxTreeEvent& event);
		void OnTreeCtrl_PlayListEndDrag(wxTreeEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnTreeCtrl_PlayListKeyDown(wxTreeEvent& event);
		void OnNotebook1PageChanged(wxNotebookEvent& event);
		void OnButton_AddFSEQClick(wxCommandEvent& event);
		void OnButton_FSEQVideoClick(wxCommandEvent& event);
		void OnButton_AddAudioClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnButton_CloneClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
