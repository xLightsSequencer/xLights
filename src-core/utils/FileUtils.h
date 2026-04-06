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

namespace FileUtils
{
	// File path resolution — resolves missing/moved files by searching show dir, media dirs, etc.
	std::string FixFile(const std::string& showDir, const std::string& file);
	void SetFixFileShowDir(const std::string& showDir);
	void SetFixFileDirectories(const std::list<std::string>& dirs);
	std::string MakeRelativeFile(const std::string& file);
	void ClearNonExistentFiles();
	bool IsFileInShowDir(const std::string& showDir, const std::string& filename);
	std::string FixEffectFileParameter(const std::string& paramname, const std::string& paramvalue, const std::string& showDir);
	bool DeleteDirectory(std::string directory);

	std::string GetResourcesDir();
    void SetResourcesDir(const std::string& dir);
};