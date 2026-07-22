#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <list>
#include <optional>

namespace FileUtils
{
	// File path resolution — resolves missing/moved files by searching show dir, media dirs, etc.
	std::string FixFile(const std::string& showDir, const std::string& file);
	void SetFixFileShowDir(const std::string& showDir);
	void SetFixFileDirectories(const std::list<std::string>& dirs);
	std::string MakeRelativeFile(const std::string& file);

	// MakeRelativeFile, falling back to the path as given when it is not under
	// the show or a media directory. This is the form file paths are stored in.
	std::string MakeRelativeFileOrOriginal(const std::string& file);

	// Filename component of a path, splitting on both '/' and '\'
	std::string GetFilenameFromPath(const std::string& path);

	// True for a path that is absolute or rooted under *either* platform's rules
	// — POSIX "/...", Windows "\...", "H:\..." or UNC "\\server\...". Show files
	// move between Windows/macOS/Linux, so std::filesystem::path::is_absolute()
	// alone (which only understands the host's form) is not enough to tell a
	// portable show-relative path from a foreign absolute one.
	bool IsAbsoluteOrRootedPath(const std::string& path);

	void ClearNonExistentFiles();
	bool IsFileInShowDir(const std::string& showDir, const std::string& filename);
	std::string FixEffectFileParameter(const std::string& paramname, const std::string& paramvalue, const std::string& showDir);
	bool DeleteDirectory(std::string directory);

	std::string GetResourcesDir();
    void SetResourcesDir(const std::string& dir);

    // Resolves the effectmetadata directory from GetResourcesDir() (plus
    // dev-build fallback locations). Returns "" if GetResourcesDir() is unset
    // or no candidate exists; the empty result is not cached, so it retries
    // once the resources dir has been established.
    std::string GetEffectMetadataDirectory();

    // Returns the file's last-write-time as Unix epoch seconds
    std::optional<long long> GetFileModTimeTicks(const std::string& path);
    bool NeedsBaseFileUpdate(const std::string& path, const std::string& syncedTicks, const std::string& mergeDescription);
};
