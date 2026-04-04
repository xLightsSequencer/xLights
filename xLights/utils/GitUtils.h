#ifndef GITUTILS_H
#define GITUTILS_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_set>

struct GitUtils
{
	static GitUtils& Instance();

	[[nodiscard]] bool IsGitInstalled() const;

	void SetFolder(std::string folder);

	[[nodiscard]] bool IsFolderInGit() const;

	[[nodiscard]] std::string GetLocalBranch() const;

	[[nodiscard]] std::string GetRemoteBranch() const;

	[[nodiscard]] std::string GetLocalHash() const;

	[[nodiscard]] std::string CalcShortHash() const;

	[[nodiscard]] bool HasLocalChanges() const;

	bool StashChanges() const;

	bool UnStashChanges() const;

	bool AddFile(std::string const& filePath);

private:
	GitUtils();

	bool findGitInstallLocation() const;

	bool checkIfFolderIsInGit();

	std::string RunGitCommand(std::string const& cmd) const;
	std::string RunGitCommandInFolder(std::string const& cmd) const;

	bool m_hasGit{ false };
	bool m_folderInGit{ false };
	std::string m_folder;
	std::string m_localBranch;
	std::string m_remoteBranch;
	std::string m_hash;

	std::unordered_set<std::string> m_filesInGit;
};

#endif
