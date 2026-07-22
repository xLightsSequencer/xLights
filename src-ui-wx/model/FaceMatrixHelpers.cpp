/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "FaceMatrixHelpers.h"

#include "MatrixFaceDownloadDialog.h"
#include "shared/utils/wxUtilities.h"

#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/msgdlg.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include <log.h>

namespace FaceMatrixHelpers
{

const std::list<std::string>& Phonemes()
{
    static const std::list<std::string> phonemes = { "AI", "E", "etc", "FV", "L", "MBP", "O", "rest", "U", "WQ" };
    return phonemes;
}

const wxString& SupportedImageTypesFilter()
{
    static const wxString filter = "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                   ";*.webp"
                                   "|All files (*.*)|*.*";
    return filter;
}

std::string GenerateKey(int col, const std::string& phoneme)
{
    if (col == 0) {
        return "Mouth-" + phoneme + "-EyesOpen";
    }
    return "Mouth-" + phoneme + "-EyesClosed";
}

std::string ExtractKey(const wxString& key)
{
    return key.AfterFirst('-').BeforeFirst('-').ToStdString();
}

std::list<std::string> GetPhonemes(const std::string& key)
{
    if (key == "AI") {
        return { "AI", "Ai", "ai", "A", "a", "A,I", "a,i" };
    } else if (key == "E") {
        return { "E", "e" };
    } else if (key == "etc") {
        return { "etc", "ETC", "Etc", "ect", "ECT", "Ect" };
    } else if (key == "FV") {
        return { "FV", "Fv", "fv", "F", "f", "F,V", "f,v" };
    } else if (key == "L") {
        return { "L", "l" };
    } else if (key == "MBP") {
        return { "MBP", "Mbp", "mbp" };
    } else if (key == "O") {
        return { "O", "o" };
    } else if (key == "rest") {
        return { "rest", "Rest", "REST" };
    } else if (key == "U") {
        return { "U", "u" };
    } else if (key == "WQ") {
        return { "WQ", "wq", "Wq", "W", "w", "W,Q", "w,q" };
    }
    wxASSERT(false);
    return {};
}

// replace the count'th occurrence of key with phoneme and return the new name
wxFileName GetFileNamePhoneme(const wxFileName& fn, const std::string& key, int count, const std::string& phoneme)
{
    wxString base = fn.GetName();
    for (int i = 0; i <= count - 1; i++) {
        base.Replace(key, ":", false);
    }
    base.Replace(key, phoneme, false);
    base.Replace(":", key);

    wxFileName fn2(fn);
    fn2.SetName(base);

    return fn2;
}

std::string FixPhonemeCase(const std::string& p)
{
    wxString pp = wxString(p).Lower();

    if (pp == "ai") return "AI";
    if (pp == "etc") return "etc";
    if (pp == "fv") return "FV";
    if (pp == "wq") return "WQ";
    if (pp == "rest") return "rest";
    if (pp == "u") return "U";
    if (pp == "o") return "O";
    if (pp == "mbp") return "MBP";
    if (pp == "e") return "E";
    if (pp == "l") return "L";

    return "";
}

bool IsValidPhoneme(const std::string& phoneme)
{
    for (const auto& p : Phonemes()) {
        if (wxString(p).Lower() == wxString(phoneme).Lower()) return true;
    }
    return false;
}

int GetRowForPhoneme(const std::string& phoneme)
{
    int row = 0;
    for (const auto& p : Phonemes()) {
        if (wxString(p).Lower() == wxString(phoneme).Lower()) return row;
        ++row;
    }
    return -1;
}

static void DoSetPhonemes(const wxFileName& fn, const std::string& actualkey, const std::string& key, int count, int row, int col,
                          const std::list<std::string>& phonemes, const std::string& setPhoneme,
                          const std::function<std::string(const std::string& key)>& getter,
                          const std::function<void(const std::string& key, int row, int col, const std::string& value)>& setter)
{
    if (key == setPhoneme) return;

    for (const auto& variant : phonemes) {
        wxFileName fn2 = GetFileNamePhoneme(fn, actualkey, count, variant);
        std::string current = getter(GenerateKey(col, setPhoneme));
        if (FileExists(fn2) && (current == "" || !FileExists(current))) {
            setter(GenerateKey(col, setPhoneme), row, col, fn2.GetFullPath().ToStdString());
        }
    }
}

static void DoSetMatrixModels(const wxFileName& fn, const std::string& actualkey, const std::string& key, int count, int col,
                              const std::function<std::string(const std::string& key)>& getter,
                              const std::function<void(const std::string& key, int row, int col, const std::string& value)>& setter)
{
    int i = 0;
    for (const auto& phoneme : Phonemes()) {
        DoSetPhonemes(fn, actualkey, key, count, i++, col, GetPhonemes(phoneme), phoneme, getter, setter);
    }
}

void AutoFillMatrixPhonemes(const std::string& pickedFile, const std::string& fullKey, int col,
                            const std::function<std::string(const std::string& key)>& getter,
                            const std::function<void(const std::string& key, int row, int col, const std::string& value)>& setter)
{
    if (pickedFile.empty()) {
        return;
    }
    wxFileName fn = wxFileName(pickedFile);

    std::string k = ExtractKey(fullKey);

    auto phonemes = GetPhonemes(k);

    bool done = false;
    // try each of the possible variants in the phoneme
    for (auto it = phonemes.begin(); !done && it != phonemes.end(); ++it) {
        int replacecount = fn.GetName().Replace(*it, "etc", true);

        // because some file systems are case sensitive try some common variants
        for (int i = 0; i < replacecount; i++) {
            // get list of etc Phonemes
            const std::list<std::string> findList = GetPhonemes("etc");
            // loop through and find the "etc" file
            for (const auto& phen : findList) {
                const wxFileName fn2 = GetFileNamePhoneme(fn, *it, i, phen);
                if (FileExists(fn2)) {
                    DoSetMatrixModels(fn, *it, k, i, col, getter, setter);
                    done = true;
                    break;
                }
            }
        }
    }
}

bool DownloadFaceImages(wxWindow* parent, const std::string& showDir, int bufferWi, int bufferHt,
                        const std::function<void(const std::string& key, int row, int col, const std::string& value)>& setter)
{
    if (showDir == "") {
        wxMessageBox("Show folder is not valid. Face image download aborted.");
        return false;
    }

    MatrixFaceDownloadDialog dlg(parent);
    if (!dlg.DlgInit(bufferWi, bufferHt)) {
        return false;
    }
    if (dlg.ShowModal() != wxID_OK) {
        return false;
    }

    std::string faceZip = dlg.GetFaceFile();

    // create folder if necessary
    wxString const dir = wxString(showDir) + wxFileName::GetPathSeparator() + "DownloadedFaces";
    if (!wxDir::Exists(dir)) {
        wxMkDir(dir, wxS_DIR_DEFAULT);
    }

    std::list<std::string> files;

    // extract all the files in the zip file into that directory
    wxFileInputStream fin(faceZip);
    wxZipInputStream zin(fin);
    wxZipEntry* ent = zin.GetNextEntry();
    while (ent != nullptr) {
        if (ent->IsDir()) {
            wxString dirname = dir + wxFileName::GetPathSeparator() + ent->GetName();
            if (!wxDirExists(dirname)) {
                spdlog::debug("Extracting dir {}:{} to {}.", (const char*)faceZip.c_str(), (const char*)ent->GetName().c_str(), (const char*)dirname.c_str());
                wxFileName::Mkdir(dirname, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
            }
        } else {
            wxString filename = dir + wxFileName::GetPathSeparator() + ent->GetName();
            files.push_back(filename);

            if (!FileExists(filename)) {
#ifdef __WXMSW__
                if (filename.length() > MAX_PATH) {
                    spdlog::warn("Target filename longer than {} chars ({}). This will likely fail. {}.", MAX_PATH, (int)filename.length(), (const char*)filename.c_str());
                }
#endif

                spdlog::debug("Extracting {}:{} to {}.", (const char*)faceZip.c_str(), (const char*)ent->GetName().c_str(), (const char*)filename.c_str());
                wxFileOutputStream fout(filename);
                zin.Read(fout);
            }
            if (!FileExists(filename)) {
                spdlog::error("File extract failed.");
            }
        }
        ent = zin.GetNextEntry();
    }

    bool applied = false;
    bool error = false;
    for (const auto& it : files) {
        wxFileName fn(it);
        wxString basefn = fn.GetName().Lower();
        bool eyesclosed = false;
        if (basefn.EndsWith("_eo")) {
            basefn = basefn.SubString(0, basefn.Length() - 4);
            eyesclosed = false;
        } else if (basefn.EndsWith("_ec")) {
            basefn = basefn.SubString(0, basefn.Length() - 4);
            eyesclosed = true;
        }

        std::string phoneme = FixPhonemeCase(basefn.AfterLast('_').ToStdString());

        if (phoneme == "" || !IsValidPhoneme(phoneme)) {
            spdlog::warn("Phoneme '{}' was not known. File {} ignored.", phoneme, it);
            error = true;
        } else {
            std::string key = "Mouth-" + phoneme + "-" + (eyesclosed ? "EyesClosed" : "EyesOpen");
            setter(key, GetRowForPhoneme(phoneme), eyesclosed ? 1 : 0, it);
            applied = true;
        }
    }

    if (error) {
        DisplayError("One or more images could not be mapped to the model due to issues with the names of files within the zip file. See log for details.", parent);
    }

    return applied;
}

} // namespace FaceMatrixHelpers
