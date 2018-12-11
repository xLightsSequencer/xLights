#ifndef PLAYLISTDIALOG_H
#define PLAYLISTDIALOG_H

//(*Headers(PlayListDialog)
#include <wx/treectrl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/filedlg.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OutputManager;
class PlayList;
class wxXmlNode;
class PlayListStep;
class PlayListItem;

class PlayListDialog: public wxDialog
{
    PlayList* _playlist;
    PlayList* _savedState;
    OutputManager* _outputManager;
    bool _dragging;
    void ValidateWindow();
    void PopulateTree(PlayList* playlist, PlayListStep* step, PlayListItem* item);
    int GetPos(const wxTreeItemId& item);
    void HighlightDropItem(wxTreeItemId* id);
    void DeleteSelectedItem();
    void SwapPage(wxNotebookPage* newpage, const std::string& text = "");
    wxTreeItemId FindStepTreeItem(PlayListStep* step);
    void AddItem(PlayList* playlist, PlayListStep* step, PlayListItem* newitem);

    public:

		PlayListDialog(wxWindow* parent, OutputManager* outputManager, PlayList* playlist, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
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
		wxFileDialog* FileDialog1;
		wxButton* Button_AddAudio;
		wxButton* Button_FSEQVideo;
		wxButton* Button_Delete;
		wxButton* Button_Cancel;
		wxButton* Button_AddFSEQ;
		wxPanel* Panel2;
		wxSplitterWindow* SplitterWindow1;
		//*)

	protected:

		//(*Identifiers(PlayListDialog)
		static const long ID_TREECTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_BUTTON6;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		static const long ID_PANEL1;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

        static const long ID_MNU_ADDSTEP;
        static const long ID_MNU_ADDESEQ;
        static const long ID_MNU_ADDFADE;
        static const long ID_MNU_ADDFSEQ;
        static const long ID_MNU_ADDSCREENMAP;
        static const long ID_MNU_ADDTEXT;
        static const long ID_MNU_ADDFILE;
        static const long ID_MNU_ADDFSEQVIDEO;
        static const long ID_MNU_ADDTEST;
        static const long ID_MNU_ADDMICROPHONE;
        static const long ID_MNU_ADDRDS;
        static const long ID_MNU_ADDPROJECTOR;
        static const long ID_MNU_ADDALLOFF;
        static const long ID_MNU_ADDSETCOLOUR;
        static const long ID_MNU_ADDCOMMAND;
        static const long ID_MNU_ADDOSC;
        static const long ID_MNU_ADDPROCESS;
        static const long ID_MNU_ADDCURL;
        static const long ID_MNU_ADDSERIAL;
        static const long ID_MNU_ADDFPPEVENT;
        static const long ID_MNU_ADDVIDEO;
        static const long ID_MNU_ADDAUDIO;
        static const long ID_MNU_ADDIMAGE;
        static const long ID_MNU_ADDJUKEBOX;
        static const long ID_MNU_ADDDELAY;
        static const long ID_MNU_ADDDIM;
        static const long ID_MNU_DELETE;
        static const long ID_MNU_REMOVEEMPTYSTEPS;

        bool IsPlayList(wxTreeItemId id);
        bool IsPlayListStep(wxTreeItemId id);
        void OnTreeCtrlMenu(wxCommandEvent &event);
        void OnDropFiles(wxDropFilesEvent& event);

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
		void OnButton_AddFSEQClick(wxCommandEvent& event);
		void OnButton_FSEQVideoClick(wxCommandEvent& event);
		void OnButton_AddAudioClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
