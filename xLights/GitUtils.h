#ifndef GITUTILS_H
#define GITUTILS_H

#include <wx/string.h>
#include "wx/frame.h"

#include <vector>
#include <unordered_set>

struct GitUtils
{
	static GitUtils& Instance();

	[[nodiscard]] bool IsGitInstalled() const;

	void SetFolder(wxString folder);

	[[nodiscard]] bool IsFolderInGit() const;

	[[nodiscard]] wxString GetLocalBranch() const;

	[[nodiscard]] wxString GetRemoteBranch() const;

	[[nodiscard]] wxString GetLocalHash() const;

	[[nodiscard]] wxString CalcShortHash() const;

	[[nodiscard]] bool HasLocalChanges() const;

	bool Fetch(wxFrame* frame);

	bool PullAndRebase(wxFrame* frame);

	bool Commit(wxFrame* frame);

	bool Push(wxFrame* frame);

	bool Status(wxFrame* frame);

	bool Reset(wxFrame* frame, bool hard = false);

	bool StashChanges() const;

	bool UnStashChanges() const;

	bool AddFile(wxString const& filePath);

	bool RunCommands(std::vector<wxString> const& commands, wxFrame* frame);

	bool RunCommand(wxString const& command, wxFrame* frame)
	{
		return RunCommands({ command }, frame);
	}


private:
	GitUtils();

	bool findGitInstallLocation() const;

	bool checkIfFolderIsInGit();

	bool m_hasGit{ false };
	bool m_folderInGit{ false };
	wxString m_folder;
	wxString m_localBranch;
	wxString m_remoteBranch;
	wxString m_hash;

	std::unordered_set<wxString> m_filesInGit;
};

#endif