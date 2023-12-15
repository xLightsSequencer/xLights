#pragma once

#include <list>
#include <string>

class TempFileManager
{
    std::list<std::string> _tempFiles;

public:
    TempFileManager()
    {}
    virtual ~TempFileManager();
    static TempFileManager& TempFileManager::GetTempFileManager();
    void AddTempFile(const std::string& file);
    void CleanupTempFiles();
};
