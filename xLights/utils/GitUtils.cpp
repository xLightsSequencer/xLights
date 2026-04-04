#include "GitUtils.h"

#include <cstdio>
#include <array>
#include <regex>
#include <format>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#include <log.h>

GitUtils& GitUtils::Instance()
{
	static GitUtils git;
	return git;
}

GitUtils::GitUtils()
{
	//look for Git installed
	//maybe a factory for Win/OSX/Linux
	if (findGitInstallLocation()) {
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

std::string GitUtils::GetLocalBranch() const
{
	return m_localBranch;
}

std::string GitUtils::GetRemoteBranch() const
{
	return m_remoteBranch;
}

std::string GitUtils::GetLocalHash() const
{
	if (!m_folderInGit) {
		return {};
	}
	return m_hash;
}

void GitUtils::SetFolder(std::string folder)
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

bool GitUtils::HasLocalChanges() const
{
	if (!m_folderInGit) {
		return false;
	}

	std::string output = RunGitCommandInFolder("git status --short --untracked-files=no");
	if (!output.empty()) {
		return true;
	}
	return false;
}

bool GitUtils::StashChanges() const
{
	if (!m_folderInGit) {
		return false;
	}

	std::string output = RunGitCommandInFolder("git stash");
	// popen doesn't give us the exit code easily via output alone,
	// but a non-empty output from git stash typically means success
	return !output.empty();
}

bool GitUtils::UnStashChanges() const
{
	if (!m_folderInGit) {
		return false;
	}

	std::string output = RunGitCommandInFolder("git stash apply");
	return !output.empty();
}

std::string GitUtils::CalcShortHash() const
{
	if (!m_folderInGit) {
		return {};
	}

	std::string output = RunGitCommandInFolder("git rev-parse --short=7 HEAD");
	// Trim trailing newline
	while (!output.empty() && (output.back() == '\n' || output.back() == '\r')) {
		output.pop_back();
	}
	return output;
}

bool GitUtils::AddFile(std::string const& filePath)
{
	if (!m_folderInGit) {
		return false;
	}

	//create list and don't re-add files
	if (m_filesInGit.find(filePath) != m_filesInGit.end()) {
		return true;
	}
	m_filesInGit.insert(filePath);

	std::string cmd = std::format("cd \"{}\" && git add \"{}\"", m_folder, filePath);
	std::string output = RunGitCommand(cmd);
	// git add produces no output on success
	return true;
}

bool GitUtils::findGitInstallLocation() const
{
	std::string output = RunGitCommand("git --version");

	static const std::regex ver_regex("version [0-9]+\\.[0-9]+\\.[0-9]+");
	if (std::regex_search(output, ver_regex)) {
		return true;
	}

	if (!output.empty()) {
		spdlog::debug("Error: {}.", output);
	}

	return false;
}

bool GitUtils::checkIfFolderIsInGit()
{
	std::string output = RunGitCommandInFolder("git status --short --branch --untracked-files=no");

	if (output.empty()) {
		return false;
	}

	// Get the first line
	std::string firstLine = output.substr(0, output.find('\n'));

	// Remove "## " prefix
	if (firstLine.size() > 3 && firstLine.substr(0, 3) == "## ") {
		firstLine = firstLine.substr(3);
	}

	// Replace "..." with "="
	std::string::size_type pos = firstLine.find("...");
	if (pos != std::string::npos) {
		firstLine.replace(pos, 3, "=");
	}

	// Split on '='
	std::string::size_type eqPos = firstLine.find('=');
	if (eqPos != std::string::npos) {
		m_localBranch = firstLine.substr(0, eqPos);
		m_remoteBranch = firstLine.substr(eqPos + 1);
	} else {
		m_localBranch = firstLine;
	}

	return true;
}

std::string GitUtils::RunGitCommand(std::string const& cmd) const
{
	std::array<char, 256> buffer;
	std::string result;
	FILE* pipe = popen(cmd.c_str(), "r");
	if (!pipe) {
		return {};
	}
	while (fgets(buffer.data(), buffer.size(), pipe)) {
		result += buffer.data();
	}
	pclose(pipe);
	return result;
}

std::string GitUtils::RunGitCommandInFolder(std::string const& cmd) const
{
	std::string fullCmd = std::format("cd \"{}\" && {}", m_folder, cmd);
	return RunGitCommand(fullCmd);
}
