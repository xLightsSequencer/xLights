#include "GitUtils.h"

#include "xLightsMain.h"
#include "xLightsApp.h"
#include "ConvertLogDialog.h"

#include <log4cpp/Category.hh>

#include <wx/utils.h>
#include <wx/regex.h>
#include "wx/process.h"
#include "wx/txtstrm.h"

GitUtils& GitUtils::Instance()
{
	static GitUtils git;
	return git;
}

GitUtils::GitUtils()
{
	//look for Git installed 
	//maybe a factory for Win/OSX/Linux
	if(findGitInstallLocation()) {
		m_hasGit = true;
	}
}

bool GitUtils::IsGitInstalled() const
{
	return m_hasGit;
}

bool GitUtils::IsFolderInGit() const
{
	return m_folderInGit;
}

wxString GitUtils::GetLocalBranch() const
{
	return m_localBranch;
}

wxString GitUtils::GetRemoteBranch() const
{
	return m_remoteBranch;
}

wxString GitUtils::GetLocalHash() const
{
	if (!m_folderInGit) {
		return wxString();
	}
	return wxString::Format("%s", m_hash);
}

void GitUtils::SetFolder(wxString folder) 
{
	m_folder = std::move(folder);
	if (!m_hasGit) {
		m_folderInGit = false;
		return;
	}
	m_filesInGit.clear();
	m_localBranch.clear();
	m_remoteBranch.clear();
	m_folderInGit = checkIfFolderIsInGit();
	m_hash = CalcShortHash();
}

bool GitUtils::Fetch(wxFrame* frame)
{
	if (!m_folderInGit) {
		return false;
	}
	return RunCommand("git fetch --verbose", frame);
}

bool GitUtils::PullAndRebase(wxFrame* frame)
{
	if(!m_folderInGit) {
		return false;
	}

	wxString cmds = "git pull --rebase --verbose";

	if (HasLocalChanges()) {
		if (wxMessageBox("The Local folder has Pending Changes\n Would you like to Stash them first?", 
			"Pending Changes?", wxYES_NO | wxCENTER,  frame) == wxYES) {
			cmds += " --autostash";
		}
	}
	return RunCommand(cmds, frame);
}

bool GitUtils::Commit(wxFrame* frame)
{
	if (!m_folderInGit) {
		return false;
	}

	if (!HasLocalChanges()) {
		wxMessageBox("No Changes Found to Commit", "Commit Error", wxOK | wxCENTRE, frame);
		return false;
	}

	wxTextEntryDialog dlg(frame, "Commit Message", "Commit Message","", wxTE_MULTILINE);
	OptimiseDialogPosition(&dlg);
	if (dlg.ShowModal() == wxID_OK) {
		wxString cmd = wxString::Format("git commit --verbose -m \"%s\"", dlg.GetValue());
		return RunCommand(cmd, frame);
	}

	return false;
}

bool GitUtils::Push(wxFrame* frame)
{
	if (!m_folderInGit) {
		return false;
	}

	return RunCommand("git push --verbose" , frame);
}

bool GitUtils::Status(wxFrame* frame)
{
	if (!m_folderInGit) {
		return false;
	}
	return RunCommand("git status --verbose", frame);
}

bool GitUtils::Reset(wxFrame* frame, bool hard)
{
	if (!m_folderInGit) {
		return false;
	}

	wxString cmd = "git reset";

	if (hard) {
		cmd += " --hard";
	}

	return RunCommand(cmd, frame);
}

bool GitUtils::HasLocalChanges() const
{
	if (!m_folderInGit) {
		return false;
	}

	wxExecuteEnv parm;
	parm.cwd = m_folder;
	wxArrayString output, errors;
	int code = wxExecute(wxT("git status --short --untracked-files=no"), output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE, &parm);
	if (code == 0 && output.size() > 0) {
		return true;
	}

	if (errors.size() > 0)
	{
		static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.debug("Error: %s.", (const char*)errors[0].c_str());
	}
	return false;
}

bool GitUtils::StashChanges() const 
{
	if (!m_folderInGit) {
		return false;
	}

	wxExecuteEnv parm;
	parm.cwd = m_folder;
	wxArrayString output, errors;
	int code = wxExecute(wxT("git stash"), output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE, &parm);
	if (code == 0) {
		return true;
	}

	if (errors.size() > 0) {
		static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.debug("Error: %s.", (const char*)errors[0].c_str());
	}
	return false;
}

bool GitUtils::UnStashChanges() const
{
	if (!m_folderInGit) {
		return false;
	}

	wxExecuteEnv parm;
	parm.cwd = m_folder;
	wxArrayString output, errors;
	int code = wxExecute(wxT("git stash apply"), output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE, &parm);
	if (code == 0) {
		return true;
	}

	if (errors.size() > 0) {
		static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.debug("Error: %s.", (const char*)errors[0].c_str());
	}
	return false;
}

wxString GitUtils::CalcShortHash() const
{
	if (!m_folderInGit) {
		return wxString();
	}

	wxExecuteEnv parm;
	parm.cwd = m_folder;
	wxArrayString output, errors;
	int code = wxExecute(wxT("git rev-parse --short=7 HEAD"), output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE, &parm);
	if (output.size() > 0) {
		return output[0];
	}
	return wxString();
}

bool GitUtils::AddFile(wxString const& filePath)
{
	if(!m_folderInGit) {
		return false;
	}

	if (!xLightsApp::GetFrame()->IsInShowFolder(filePath)) {
		return false;
	}

	//create list and don't re-add files
	if (m_filesInGit.find(filePath) != m_filesInGit.end()) {
		return true;
	}
	m_filesInGit.insert(filePath);

	wxExecuteEnv parm;
	parm.cwd = m_folder;

	wxString const cmd = wxString::Format("git add \"%s\"", filePath);
	wxArrayString output, errors;
	int code = wxExecute(cmd, output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE, &parm);

	if (code == 0 ) {
		return true;
	}
	if(errors.size() > 0) {
		static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.debug("Error: %s.", (const char*)errors[0].c_str());
	}

	return false;
}

bool GitUtils::findGitInstallLocation() const
{
	wxArrayString output, errors;
	int code = wxExecute(wxT("git --version"), output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE);

	static wxRegEx ver_regex(" (version [0-9]+.[0-9]+.[0-9]+)", wxRE_ADVANCED | wxRE_NEWLINE);
	if (code == 0 && output.size() > 0 && ver_regex.Matches(output[0])) {
		return true;
	}

	if (errors.size() > 0) {
		static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.debug("Error: %s.", (const char*)errors[0].c_str());
	}

	return false;
}

bool GitUtils::RunCommands(std::vector<wxString> const& commands, wxFrame* frame)
{
	bool value = true;
	wxExecuteEnv parm;
	parm.cwd = m_folder;

	ConvertLogDialog* logDisp = new ConvertLogDialog(frame);
	logDisp->SetLabel("Git Log");

	logDisp->Show();
	wxProcess* process = new wxProcess(wxPROCESS_REDIRECT);
	int returnCode = -1;
	process->Bind(wxEVT_END_PROCESS, [&returnCode](wxProcessEvent& event) {
		returnCode = event.GetExitCode();
	});

	for (auto const& cmd : commands) {
		returnCode = -1;
		long pid = wxExecute(cmd, wxEXEC_ASYNC, process, &parm);
		process->Redirect();

		if (process) {
			wxInputStream* msg = process->GetInputStream();

			wxTextInputStream tStream(*msg);
			while (!msg->Eof()) {
				wxString log = tStream.ReadLine();
				logDisp->AppendConvertStatus(log + wxT("\n"));
			}

			process->CloseOutput();

			while (returnCode == -1) {
				wxMilliSleep(10);
				::wxSafeYield();//needed to process end event
			}
			if (returnCode != 0) {
				value = false;
			}
		} else {
			value = false;
		}
	}
	m_hash = CalcShortHash();

	logDisp->AppendConvertStatus(wxString::Format("Folder:'%s' Branch:'%s' %s\n", m_folder, GetLocalBranch(), GetLocalHash()));
	logDisp->AppendConvertStatus(wxT("Finished!\n"));
	process->CloseOutput();
	process = nullptr;
	delete process;
	return value;
}

bool GitUtils::checkIfFolderIsInGit()
{
	wxExecuteEnv parm;
	parm.cwd = m_folder;

	wxArrayString output, errors;
	int code = wxExecute(wxT("git status --short --branch --untracked-files=no"), output, errors, wxEXEC_BLOCK | wxEXEC_HIDE_CONSOLE | wxEXEC_NODISABLE, &parm);

	if (code == 0 && output.size() > 0) {
		wxString message = output[0];
		message.Replace("## ", "");
		message.Replace("...", "=");
		auto const& parts = wxSplit(message, '=');
		m_localBranch = parts[0];

		if (parts.size() > 1) {
			m_remoteBranch = parts[1];
		}
		return true;
	}

	if (errors.size() > 0) {
		static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
		logger_base.debug("Error: %s.", (const char*)errors[0].c_str());
	}

	return false;
}
