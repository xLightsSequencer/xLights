#include "TempFileManager.h"

#include <wx/wx.h>
#include <wx/file.h>

TempFileManager __tempFileManager;

TempFileManager& TempFileManager::GetTempFileManager()
{
    return __tempFileManager;
}

TempFileManager::~TempFileManager()
{
    CleanupTempFiles();
}

void TempFileManager::AddTempFile(const std::string& file)
{
    _tempFiles.push_back(file);
}

void TempFileManager::CleanupTempFiles()
{
    for (const auto& it : _tempFiles) {
        if (wxFile::Exists(it)) {
            wxRemoveFile(it);
        }
    }
    _tempFiles.clear();
}