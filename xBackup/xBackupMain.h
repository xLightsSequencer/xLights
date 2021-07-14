/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#ifdef _MSC_VER

#include <stdlib.h>

//#define VISUALSTUDIO_MEMORYLEAKDETECTION
#ifdef VISUALSTUDIO_MEMORYLEAKDETECTION
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#endif

//(*Headers(xBackupFrame)
#include <wx/button.h>
#include <wx/frame.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/statusbr.h>
#include <wx/textctrl.h>
//*)

#include <list>
#include <vector>
#include <wx/socket.h>
#include <wx/treelist.h>
#include <wx/file.h>
#include <wx/dir.h>

class wxDebugReportCompress;
struct Model;
struct Controller;

class xBackupFrame : public wxFrame
{
public:
        xBackupFrame(wxWindow* parent, wxWindowID id = -1);
        virtual ~xBackupFrame();
        void SendReport(const wxString &loc, wxDebugReportCompress &report);
        void CreateDebugReport(wxDebugReportCompress* report);

private:

        //(*Handlers(xBackupFrame)
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnButtonRunClick(wxCommandEvent& event);
        void OnMenuItem_ShowFolderSelected(wxCommandEvent& event);
        void OnMenuItem_LogSelected(wxCommandEvent& event);
        void OnListBoxBackupsSelect(wxCommandEvent& event);
        void OnMenuItem_Wipe_xLights_SettingsSelected(wxCommandEvent& event);
        //*)

        //(*Identifiers(xBackupFrame)
        static const long ID_LISTBOX_BACKUPS;
        static const long ID_TEXTCTRL_BACKUP_DATA;
        static const long ID_BUTTON_RUN;
        static const long ID_STATICTEXT_BACKUPFOLDER;
        static const long ID_STATICTEXT_SHOWFOLDER;
        static const long ID_STATUSBAR1;
        static const long ID_MNU_SHOWFOLDER;
        static const long ID_MNU_WIPE_XLIGHTS_SETTINGS;
        static const long ID_MNU_QUIT;
        static const long ID_MNU_ABOUT;
        static const long ID_MNU_LOG;
        //*)

        //(*Declarations(xBackupFrame)
        wxButton* ButtonRun;
        wxListBox* ListBoxBackups;
        wxMenu* Menu1;
        wxMenu* Menu2;
        wxMenuBar* MenuBar1;
        wxMenuItem* MenuItem_About;
        wxMenuItem* MenuItem_Log;
        wxMenuItem* MenuItem_Quit;
        wxMenuItem* MenuItem_ShowFolder;
        wxMenuItem* MenuItem_Wipe_xLights_Settings;
        wxStaticText* StaticTextBackUpFolder;
        wxStaticText* StaticTextShowFolder;
        wxStatusBar* StatusBar1;
        wxTextCtrl* TextCtrl_BackupData;
        //*)
        void OnChar(wxKeyEvent& event);

        DECLARE_EVENT_TABLE()

        wxString _showDir;
        wxString _backupDir;

        [[nodiscard]] wxString xLightsShowDir() const;
        void SetBackupDir();
        void ListBackupDir();

        void BackUpShowDir(wxString const& folder);
        void RestoreBackUp(wxString const& showfolder, wxString const& backupfolder);

        void PopulateDataBox(wxString const& folder);
        [[nodiscard]] wxArrayString GetSeqList(wxString const& folder) const;
        [[nodiscard]] std::vector<Controller> LoadNetworkFile(wxString const& folder) const;
        [[nodiscard]] std::vector<Model> LoadRGBEffectsFile(wxString const& folder) const;

        bool ReadBackupFolderFromRGBEffectsFile(wxString const& folder);

        void BackupDirectory(wxString sourceDir, wxString targetDirName, wxString lastCreatedDirectory, std::string& errors);
        bool CopyFiles(const wxString& wildcard, wxDir& srcDir, wxString& targetDirName, wxString lastCreatedDirectory, std::string& errors);
        void CreateMissingDirectories(wxString targetDirName, wxString lastCreatedDirectory, std::string& errors);

        bool IsXlightsRunning() const;
        [[nodiscard]] wxArrayString GetProcessList() const;
        void ExecuteCommand(const wxString& command, wxArrayString& output, long flags = wxEXEC_NODISABLE | wxEXEC_SYNC) const;

        void DeleteFolder();
        bool RemoveDir(wxString rmDir);
};
